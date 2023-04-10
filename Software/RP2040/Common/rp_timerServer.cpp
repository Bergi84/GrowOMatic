#include "rp_timerServer.h"
#include "pico/stdlib.h"
#include "hardware/timer.h"


TTimer::TTimer(TTimerServer* aTS, uint32_t (*aCbFunc)(void* aArg), void* aArg)
{
    mTS = aTS;

    mCbFunc = aCbFunc;
    mCbArg = aArg;
    mNext = 0;
    mPrev = 0;
    mAktiv = false;
    mTimerAlarmTic = 0;
}

void TTimer::setTimer(uint32_t aTck)
{
    mTS->mAktTime = time_us_32();

    uint32_t status = save_and_disable_interrupts();

    if(mAktiv)
        mTS->unqueueTimer(this);

    if(mTS->mTimerList == 0)
        mTS->mLastTime = mTS->mAktTime;

    mTimerAlarmTic = mTS->mAktTime + aTck;

    mTS->queueTimer(this, aTck);

    restore_interrupts(status);
}

void TTimer::stopTimer()
{
    mTS->mAktTime = time_us_32();

    uint32_t status = save_and_disable_interrupts();
    mTS->unqueueTimer(this);
    restore_interrupts(status);
}

void TTimer::delTimer()
{
    stopTimer();
    delete this;    
}

void TTimerServer::init()
{
    mAlarmNum = hardware_alarm_claim_unused(true);
    mTimerList = 0;
    mIrqVeneer.init(this, irqHandler);
    setIrqHandler(mIrqVeneer.getFunc());
}

void TTimerServer::irqHandler(void* aArg)
{
    TTimerServer* pObj = (TTimerServer*) aArg;

    uint32_t status = save_and_disable_interrupts();

    timer_hw->intf &= ~(1u << pObj->mAlarmNum);

    pObj->mAktTime = time_us_32();

    gDebug.setPin(7);

    bool timerDisarmed = false;
    while(pObj->mTimerList != 0 && ((pObj->mTimerList->mTimerAlarmTic - pObj->mLastTime) <= (pObj->mAktTime - pObj->mLastTime)))
    {
        timerDisarmed = true;

        TTimer* tmp = pObj->mTimerList;

        // unqueue timer
        pObj->unqueueTimer(pObj->mTimerList, false);

        // excute call back
        uint32_t retVal = tmp->mCbFunc(tmp->mCbArg);

        // requeue timer
        if(retVal > 0)
        {
            tmp->mTimerAlarmTic += retVal;
            pObj->queueTimer(tmp, false);
        }
    }

    if(pObj->mTimerList != 0 && timerDisarmed)
    {
        // start timer
        timer_hw->alarm[pObj->mAlarmNum] = (uint32_t) pObj->mTimerList->mTimerAlarmTic;     // start timer

        // check missing timer
        uint32_t newTime = time_us_32();
        if((pObj->mTimerList->mTimerAlarmTic - pObj->mAktTime) <= (newTime - pObj->mAktTime))
        {
            // new Time is past the aktive alarm
            timer_hw->intf |= 1u << pObj->mAlarmNum;   // force interrupt
        }
    }

    gDebug.resetPin(7);

    pObj->mLastTime = pObj->mAktTime;

    restore_interrupts(status);
}

void TTimerServer::setIrqHandler(void (*aIrqHandler)())
{
    timer_hw->intr = 1u << mAlarmNum;
    timer_hw->inte = 1u << mAlarmNum;
    irq_set_exclusive_handler(TIMER_IRQ_0 + mAlarmNum, aIrqHandler);
    irq_set_priority(TIMER_IRQ_0 + mAlarmNum, PICO_DEFAULT_IRQ_PRIORITY);
    irq_set_enabled(TIMER_IRQ_0 + mAlarmNum, true);
}

void TTimerServer::queueTimer(TTimer* aTimer, bool aStartNext)
{
    aTimer->mAktiv = true;

    if(mTimerList == 0 || (aTimer->mTimerAlarmTic - mLastTime) < (mTimerList->mTimerAlarmTic - mLastTime))
    {
        // no aktive Timers or insert at the beginning of the list
        if(mTimerList != 0)
            mTimerList->mPrev = aTimer;


        aTimer->mNext = mTimerList;
        aTimer->mPrev = 0;
        mTimerList = aTimer;

        // clear interrupt signals
        timer_hw->armed = 1u << mAlarmNum;                                  // disarme timer
        timer_hw->intr = 1u << mAlarmNum;                                   // delete interrupt request

        if(aStartNext)
        {
            // start timer
            timer_hw->alarm[mAlarmNum] = (uint32_t) aTimer->mTimerAlarmTic;

            // check for missing timer
            uint32_t newTime = time_us_32();
            if((mTimerList->mTimerAlarmTic - mAktTime) <= (newTime - mAktTime))
            {
                // new Time is past the aktive alarm
                timer_hw->intf |= 1u << mAlarmNum;   // force interrupt
            }
        }
    }
    else
    {
        TTimer* tmp = mTimerList->mNext;
        TTimer* prev = mTimerList;

        while(tmp != 0 && (aTimer->mTimerAlarmTic - mLastTime) > (tmp->mTimerAlarmTic - mLastTime))
        {
            prev = tmp;
            tmp = tmp->mNext;
        }

        aTimer->mNext = tmp;
        aTimer->mPrev = prev;
        prev->mNext = aTimer;
        if(tmp != 0)
            tmp->mPrev = aTimer;
    }
}

void TTimerServer::unqueueTimer(TTimer* aTimer, bool aStartNext)
{

    if(aTimer->mAktiv)
    {
        aTimer->mAktiv = false;

        if(aTimer->mPrev == 0)
        {
            // timer is the currently aktive Timer
            mTimerList = mTimerList->mNext;
            if(mTimerList != 0)
                mTimerList->mPrev = 0;

            // start Timer
            timer_hw->armed = 1u << mAlarmNum;                                  // disarme timer
            timer_hw->intr = 1u << mAlarmNum;                                   // delete interrupt request

            if(mTimerList != 0 && aStartNext)
            {
                timer_hw->alarm[mAlarmNum] = (uint32_t) mTimerList->mTimerAlarmTic;     // start timer

                // check missing timer
                uint32_t newTime = time_us_32();
                if((mTimerList->mTimerAlarmTic - mAktTime) < (newTime - newTime))
                {
                    // new Time is past the aktive alarm
                    timer_hw->intf |= 1u << mAlarmNum;   // force interrupt

                    // *((io_rw_32 *) (PPB_BASE + M0PLUS_NVIC_ISPR_OFFSET)) = 1 << (TIMER_IRQ_0 + mAlarmNum);
                }
            }
        }
        else
        {
            aTimer->mPrev->mNext = aTimer->mNext;
            if(aTimer->mNext != 0)
                aTimer->mNext->mPrev = aTimer->mPrev;
        }
    }
}
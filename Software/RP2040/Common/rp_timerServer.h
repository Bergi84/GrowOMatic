#ifndef RP_TIMERSERVER_H_
#define RP_TIMERSERVER_H_

#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/platform.h"
#include "hardware/sync.h"
#include "rp_debug.h"

class TTimerServer;

class TTimer {
private:
    TTimer() {};
    // if cbFunc returns a Value greater than 0 value is added to last alarm time and restart again
    TTimer(TTimerServer* aTS, uint32_t (*aCbFunc)(void* aArg), void* aArg);

    friend class TTimerServer;

    TTimerServer* mTS;

    uint32_t (*mCbFunc)(void* aArg);
    void* mCbArg;

    uint32_t mTimerAlarmTic;
    bool mAktiv;

    TTimer* mNext;
    TTimer* mPrev;

public:
    void __time_critical_func(setTimer)(uint32_t aTck);
    void __time_critical_func(stopTimer)();
    void delTimer();
    bool isAktive() { return mAktiv; };
};

class TTimerServer
{
public:
    void init();
    void setIrqHandler(void (*aIrqHandler)());
    TTimer* getTimer(uint32_t (*aCbFunc)(void* aArg), void* aArg)
    {
        return new TTimer(this, aCbFunc, aArg);
    }

    inline void irqHandler()
    {
        uint32_t status = save_and_disable_interrupts();

        timer_hw->intf &= ~(1u << mAlarmNum);

        mAktTime = time_us_32();

        gDebug.setPin(7);

        bool timerDisarmed = false;
        while(mTimerList != 0 && ((mTimerList->mTimerAlarmTic - mLastTime) <= (mAktTime - mLastTime)))
        {
            timerDisarmed = true;

            TTimer* tmp = mTimerList;

            // unqueue timer
            unqueueTimer(mTimerList, false);

            // excute call back
            uint32_t retVal = tmp->mCbFunc(tmp->mCbArg);

            // requeue timer
            if(retVal > 0)
            {
                tmp->mTimerAlarmTic += retVal;
                queueTimer(tmp, false);
            }
        }

        if(mTimerList != 0 && timerDisarmed)
        {
            // start timer
            timer_hw->alarm[mAlarmNum] = (uint32_t) mTimerList->mTimerAlarmTic;     // start timer

            // check missing timer
            uint32_t newTime = time_us_32();
            if((mTimerList->mTimerAlarmTic - mAktTime) <= (newTime - mAktTime))
            {
                // new Time is past the aktive alarm
                timer_hw->intf |= 1u << mAlarmNum;   // force interrupt
            }
        }

        gDebug.resetPin(7);

        mLastTime = mAktTime;

        restore_interrupts(status);
    }

private:
    friend class TTimer;

    uint32_t mAlarmNum;
    uint32_t mLastTime;
    uint32_t mAktTime;

    TTimer* mTimerList;

    void __time_critical_func(queueTimer)(TTimer* aTimer, bool aStartNext = true);
    void __time_critical_func(unqueueTimer)(TTimer* aTimer, bool aStartNext = true);
};

#endif
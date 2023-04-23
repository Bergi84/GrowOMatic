#ifndef RP_TIMERSERVER_H_
#define RP_TIMERSERVER_H_

#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/platform.h"
#include "hardware/sync.h"
#include "rp_debug.h"
#include "irqVeneer.h"

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
    TTimer* getTimer(uint32_t (*aCbFunc)(void* aArg), void* aArg)
    {
        return new TTimer(this, aCbFunc, aArg);
    }


private:
    friend class TTimer;

    uint32_t mAlarmNum;
    uint32_t mLastTime;
    uint32_t mAktTime;

    TTimer* mTimerList;

    irqVeneer_t mIrqVeneer;

    spin_lock_t *mSpinLock;

    void __time_critical_func(queueTimer)(TTimer* aTimer, bool aStartNext = true);
    void __time_critical_func(unqueueTimer)(TTimer* aTimer, bool aStartNext = true);

    static void __time_critical_func(irqHandler)(void* aArg);
    void setIrqHandler(void (*aIrqHandler)());
};

#endif
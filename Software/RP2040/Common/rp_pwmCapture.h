#ifndef RP_PWMCAPTURE_H_
#define RP_PWMCAPTURE_H_

#include "stdint.h"
#include "rp_dmaIrqMng.h"
#include "hardware/pio.h"
#include "rp_timerServer.h"

class TPwmCapture
{
private:
    PIO mPio;
    TTimerServer* mTS;
    uint32_t mSmNo;
    uint32_t mGpio;
    uint32_t mDmaDataCh;
    uint32_t mDmaCtrlCh;
    uint32_t mMaxPeriod;
    uint32_t mDmaWriteAdr;

    uint32_t mHighCnt;
    uint32_t mLenCnt;

    TTimer* mTimoutTimer;

    static void __time_critical_func(irqCb)(void* aArg);
    static uint32_t __time_critical_func(timeOutCb)(void *aArg);

public:

    // aMax period is in us
    void init(PIO aPio, uint aSmNo, uint aGpio, uint32_t aMaxPeriod, TDmaIrqMng* aIrqMng, TTimerServer* aTS);

    // return period length in clks
    inline uint32_t getPeriod() {
        if(mLenCnt == 0)
            return 0xffffffff;
        else
            return (0xffffffff - mLenCnt) * 2 + 6 ;
    }

    // return high length in clks
    inline uint32_t getHigh() {
        return (0xffffffff - mHighCnt) * 2 + 4 ;
    }
};

#endif
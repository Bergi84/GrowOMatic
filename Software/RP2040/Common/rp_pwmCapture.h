#ifndef RP_PWMCAPTURE_H_
#define RP_PWMCAPTURE_H_

#include "stdint.h"
#include "rp_dmaIrqMng.h"
#include "hardware/pio.h"

class TPwmCapture
{
private:
    PIO mPio;
    uint32_t mSmNo;
    uint32_t mGpio;
    uint32_t mDmaCh;
    uint32_t mMaxPeriod;

    uint32_t mHighCnt;
    uint32_t mLenCnt;

    void irqCb(void* aArg);
    void timeOutCb(void *aArg);

public:
    void init(PIO aPio, uint aSmNo, uint aGpio, uint32_t aMaxPeriod, TDmaIrqMng* aIrqMng);

};

#endif
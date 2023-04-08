#ifndef RP_ADC_H_
#define RP_ADC_H_

#include "stdint.h"
#include "rp_dmaIrqMng.h"

class TAdc
{
private:
    uint16_t mResult[4];
    uint32_t mFilt[4];
    uint8_t mFiltLen[4];

    uint16_t mDmaBuf[2][4];
    uint32_t mDataDmaCh;
    uint32_t mConfDmaCh;
    uint32_t mDmaWriteAdr[2];

    uint32_t mBufInd;

public:
    void init(uint32_t aGpioMsk, TDmaIrqMng* aIrqMng);
    void setFilterLen(uint32_t aCh, uint32_t aLen)
    {
        if(aCh < 4)
            mFiltLen[aCh] = aLen;
    };
    uint16_t getResult(uint32_t aCh)
    {
        if(aCh < 4)
            return mResult[aCh];     
        else
            return 0xFFFF;
    }

    static void irqCb(void* aArg);
};

#endif
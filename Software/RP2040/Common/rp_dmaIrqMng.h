#ifndef RP_DMAIRQHANDLER_H_
#define RP_DMAIRQHANDLER_H_

#include "stdint.h"
#include "pico/stdlib.h"
#include "hardware/dma.h"

class TDmaIrqMng
{
private:
    struct {
        void (*func)(void* aArg);
        void* arg;
    } mCb[NUM_DMA_CHANNELS];
    volatile uint32_t* mStatReg;

public:
    void init(uint32_t aIrqNum, void(*aHandler)());
    void setIrq(uint32_t aCh, void (*aHandler)(void* aArg), void* aArg);

    void irqHandler()
    {
        for(int i = 0; i < NUM_DMA_CHANNELS; i++)
        {
            uint32_t msk = 1 << i;
            if((msk & *mStatReg) != 0)
            {
                if(mCb[i].func)
                    mCb[i].func(mCb[i].arg);

                *mStatReg = msk;
            }
        }
    }
};

#endif
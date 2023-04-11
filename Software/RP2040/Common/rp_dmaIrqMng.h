#ifndef RP_DMAIRQHANDLER_H_
#define RP_DMAIRQHANDLER_H_

#include "stdint.h"
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "irqVeneer.h"
#include "sequencer_armm0.h"

class TDmaIrqMng
{
private:
    struct {
        void (*func)(void* aArg);
        void* arg;
    } mCb[NUM_DMA_CHANNELS];
    volatile uint32_t* mStatReg;

    irqVeneer_t mIrqVeneer;
    static void setIrqHandler(void *aArg);
    static void __time_critical_func(irqHandler)(void* aArg);

public:
    void init(uint32_t aIrqNum, TSequencer* aSeq_c1 = 0);

    void setDmaHandler(uint32_t aCh, void (*aHandler)(void* aArg), void* aArg);
};

#endif
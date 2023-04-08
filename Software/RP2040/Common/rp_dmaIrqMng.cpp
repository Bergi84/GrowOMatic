#include "rp_dmaIrqMng.h"

void TDmaIrqMng::init(uint32_t aIrqNum, void(*aHandler)())
{
    for(int i = 0; i < NUM_DMA_CHANNELS; i++)
    {
        mCb[i].arg = 0;
        mCb[i].func = 0;
    }

    if(aIrqNum != DMA_IRQ_0 && aIrqNum != DMA_IRQ_1)
        return;

    if(DMA_IRQ_0 == aIrqNum)
    {
        mStatReg = &dma_hw->ints0;
    }
    else
    {
        mStatReg = &dma_hw->ints1;
    }

    irq_set_exclusive_handler(aIrqNum, aHandler);
    irq_set_enabled(aIrqNum, true);
}

void TDmaIrqMng::setIrq(uint32_t aCh, void (*aHandler)(void* aArg), void* aArg)
{
    mCb[aCh].func = aHandler;
    mCb[aCh].arg = aArg;

    if(mStatReg == &dma_hw->ints0)
    {
        dma_channel_set_irq0_enabled(aCh, aHandler != 0);
    }
    if(mStatReg == &dma_hw->ints1)
    {
        dma_channel_set_irq1_enabled(aCh, aHandler != 0);
    }
}
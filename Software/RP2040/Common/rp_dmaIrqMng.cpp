#include "rp_dmaIrqMng.h"

void TDmaIrqMng::init(uint32_t aIrqNum, TSequencer* aSeq_c1)
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

    mIrqVeneer.init(this, irqHandler);

    if(aSeq_c1 == 0)
    {
        irq_set_exclusive_handler(aIrqNum, mIrqVeneer.getFunc());
        irq_set_enabled(aIrqNum, true);
    }
    else
    {
        uint8_t seqId;
        aSeq_c1->addTask(seqId, setIrqHandler, this);
        aSeq_c1->queueTask(seqId);
        
        while(!aSeq_c1->taskDone(seqId));

        aSeq_c1->delTask(seqId);
    }
}

void TDmaIrqMng::setDmaHandler(uint32_t aCh, void (*aHandler)(void* aArg), void* aArg)
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

void TDmaIrqMng::irqHandler(void* aArg)
{
    TDmaIrqMng* pObj = (TDmaIrqMng*) aArg;

    for(int i = 0; i < NUM_DMA_CHANNELS; i++)
    {
        uint32_t msk = 1 << i;
        if((msk & *pObj->mStatReg) != 0)
        {
            if(pObj->mCb[i].func)
                pObj->mCb[i].func(pObj->mCb[i].arg);

            *pObj->mStatReg = msk;
        }
    }
}

void TDmaIrqMng::setIrqHandler(void *aArg)
{
    TDmaIrqMng* pObj = (TDmaIrqMng*) aArg;

    uint32_t irqNum = 0;

    if(pObj->mStatReg == &dma_hw->ints0)
    {
        irqNum = DMA_IRQ_0;
    }
    if(pObj->mStatReg == &dma_hw->ints1)
    {
        irqNum = DMA_IRQ_1;
    }

    irq_set_exclusive_handler(irqNum, pObj->mIrqVeneer.getFunc());
    irq_set_enabled(irqNum, true);
}
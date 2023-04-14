#include "rp_capSens.h"

void TCapSens::init(PIO aPio, uint aExcPinBase, uint aSensPinBase, uint aSensPinCnt, uint aSensPinPolMsk, uint aMaxChgCnt, uint aDchgCnt, TSequencer* aSeq_c1)
{
    mPio = aPio;
    mExcPinBase = aExcPinBase;
    mSensPinBase = aSensPinBase;
    mSensPinCount = aSensPinCnt;
    mSensPinPolMsk = aSensPinPolMsk;
    mMaxChgCnt = aMaxChgCnt;

    capSens_init(aPio, aExcPinBase, aSensPinBase, aSensPinCnt, aSensPinPolMsk, aMaxChgCnt, aDchgCnt);

    for(uint i = 0; i < 4; i++)
    {
        mSumBuf[i] = 0;
    }
    for(uint i = 0; i < MAX_SENSE_PIN_CNT; i++)
    {
        mCapVal[i] = 0;
    }
    setOverSampling(256);
    mSumCnt = 0;
    mAktivPin = 0;

    setNextSensPins();

    mDebEmptyCall = 0;

    mIrqVeneer.init(this, irqHandler);
    if(aSeq_c1 == 0)
    {
        setIrqHandler(this);
    }
    else
    {
        uint8_t seqId;
        aSeq_c1->addTask(seqId, setIrqHandler, this);
        aSeq_c1->queueTask(seqId);
        
        while(!aSeq_c1->taskDone(seqId));

        aSeq_c1->delTask(seqId);
    }
    pio_set_sm_mask_enabled (mPio, 0xF, true); 
}

void TCapSens::setIrqHandler(void* aArg)
{
    TCapSens* pObj = (TCapSens*) aArg;
    
    pio_set_irq0_source_enabled(pObj->mPio, pis_interrupt0 , true);
    if(pObj->mPio == pio0)
    {       
        irq_set_exclusive_handler(PIO0_IRQ_0, pObj->mIrqVeneer.getFunc());
        irq_set_enabled(PIO0_IRQ_0, true);
    }
    else
    {
        irq_set_exclusive_handler(PIO1_IRQ_0, pObj->mIrqVeneer.getFunc());
        irq_set_enabled(PIO1_IRQ_0, true);
    }
}

void TCapSens::setOverSampling(uint aSampleCnt)
{
    uint tmp = aSampleCnt - 1;
    uint cnt = 0;
    while(tmp)
    {
        tmp >>= 1;
        cnt++;
    }

    if(cnt < 4)
        cnt = 4;


    mSumShift = cnt - 4;
    mMaxShift = 4;
    mSampleCnt = 1 << cnt;
}

void TCapSens::setSensCh(uint sm, uint gpio)
{
    uint32_t tmp = mPio->sm[sm].execctrl;
    tmp &= ~PIO_SM0_EXECCTRL_JMP_PIN_BITS;
    tmp |= gpio << PIO_SM0_EXECCTRL_JMP_PIN_LSB;
    mPio->sm[sm].execctrl = tmp;
}

void TCapSens::setNextSensPins()
{
    uint tmp = mAktivPin;
    for(int i = 0; i < 4; i++)
    {
        setSensCh(i, tmp + mSensPinBase);
        if(tmp == mSensPinCount - 1)
        {
            tmp = 0;
        }
        else
        {
            tmp++;
        }
    }
}

void TCapSens::irqHandler(void *aArg)
{
    TCapSens* pObj = (TCapSens*) aArg;

    pio_interrupt_clear(pObj->mPio, 0);

    // for debugging, should never occure
    // rx fifo has no Values

    if((pObj->mPio->fstat & 0x00000F00) != 0)
    {
        pObj->mDebEmptyCall++;
        return;
    }

    for(int i = 0; i < 4; i++)
    {
        pObj->mSumBuf[i] += pObj->mPio->rxf[i];
    }

    if(pObj->mSumCnt == pObj->mSampleCnt - 1)
    {
        // stop aquisition
        pObj->mPio->irq_force = 0x02;

        // clear fifo
        while(!(pObj->mPio->fstat & 0x00000100))
            volatile uint32_t dump = pObj->mPio->rxf[0];
        while(!(pObj->mPio->fstat & 0x00000200))
            volatile uint32_t dump = pObj->mPio->rxf[1];
        while(!(pObj->mPio->fstat & 0x00000400))
            volatile uint32_t dump = pObj->mPio->rxf[2];
        while(!(pObj->mPio->fstat & 0x00000800))
            volatile uint32_t dump = pObj->mPio->rxf[3];    

        while(pObj->mPio->sm[0].addr != 20); // should be the wait instruction of master (wait 0 irq 1)

        for(int i = 0; i < 4; i++)
        {
            pObj->mCapVal[pObj->mAktivPin] = (pObj->mMaxChgCnt << pObj->mMaxShift) - (pObj->mSumBuf[i] >> pObj->mSumShift);
            pObj->mSumBuf[i] = 0;
            if(pObj->mAktivPin == pObj->mSensPinCount - 1)
            {
                pObj->mAktivPin = 0;
            }
            else
            {
                pObj->mAktivPin++;
            }
        }
        pObj->mSumCnt = 0;  

        pObj->setNextSensPins();

        // start pio
        pObj->mPio->irq = 0x02;
    }
    else
    {
        pObj->mSumCnt++;
    }
}
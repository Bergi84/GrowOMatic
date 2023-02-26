#ifndef CAPSENS_H_
#define CAPSENS_H_

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "capSens.pio.h"
#include "hardware/irq.h"

#define MAX_SENSE_PIN_CNT   32

class TCapSens {
    PIO mPio;
    uint mExcPinBase;
    uint mSensPinBase;
    uint mSensPinCount;
    uint mSensPinPolMsk;

    uint mSumBuf[4];
    uint mSumCnt;
    uint mSampleCnt;
    uint mSumShift;
    uint mMaxShift;
    uint mAktivPin;

    uint mMaxChgCnt;

    inline void setSensCh(uint sm, uint gpio)
    {
        uint32_t tmp = mPio->sm[sm].execctrl;
        tmp &= ~PIO_SM0_EXECCTRL_JMP_PIN_BITS;
        tmp |= gpio << PIO_SM0_EXECCTRL_JMP_PIN_LSB;
        mPio->sm[sm].execctrl = tmp;
    }

    inline void setNextSensPins()
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

public:
    uint mCapVal[MAX_SENSE_PIN_CNT];
    void init(PIO aPio, uint aExcPinBase, uint aSensPinBase, uint aSensPinCnt, uint aSensPinPolMsk, uint aMaxChgCnt, uint aDchgCnt)
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
    }

    void enable()
    {
        pio_set_sm_mask_enabled (mPio, 0xF, true); 
    };

    void setOverSampling(uint aSampleCnt)
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

    // this function installs the irq handler and only accepts "C" functions
    // or static memeber function. So we must provide an IRQ Handler wrapper function
    // for our capSens->irqHandler memeber function

    void setIrqHandler(void (*aIrqHandler)(void))
    {
        pio_set_irq0_source_enabled(mPio, pis_interrupt0 , true);
        if(mPio == pio0)
        {       
            irq_set_exclusive_handler(PIO0_IRQ_0, aIrqHandler);
            irq_set_enabled(PIO0_IRQ_0, true);
        }
        else
        {
            irq_set_exclusive_handler(PIO1_IRQ_0, aIrqHandler);
            irq_set_enabled(PIO1_IRQ_0, true);
        }
    }

    uint32_t mDebEmptyCall;

    inline void irqHandler()
    {
        pio_interrupt_clear(mPio, 0);

        // for debugging, should never occure
        // rx fifo has no Values

        if((mPio->fstat & 0x00000F00) != 0)
        {
            mDebEmptyCall++;
            return;
        }

        for(int i = 0; i < 4; i++)
        {
            mSumBuf[i] += mPio->rxf[i];
        }

        if(mSumCnt == mSampleCnt - 1)
        {
            // stop aquisition
            mPio->irq_force = 0x02;
            while(mPio->sm[0].addr != 20); // should be the wait instruction of master (wait 0 irq 1)

            for(int i = 0; i < 4; i++)
            {
                mCapVal[mAktivPin] = (mMaxChgCnt << mMaxShift) - (mSumBuf[i] >> mSumShift);
                mSumBuf[i] = 0;
                if(mAktivPin == mSensPinCount - 1)
                {
                    mAktivPin = 0;
                }
                else
                {
                    mAktivPin++;
                }
            }
            mSumCnt = 0;

            // clear fifo
            while(!(mPio->fstat & 0x00000100))
                volatile uint32_t dump = mPio->rxf[0];
            while(!(mPio->fstat & 0x00000200))
                volatile uint32_t dump = mPio->rxf[1];
            while(!(mPio->fstat & 0x00000400))
                volatile uint32_t dump = mPio->rxf[2];
            while(!(mPio->fstat & 0x00000800))
                volatile uint32_t dump = mPio->rxf[3];      

            setNextSensPins();

            // start pio
            mPio->irq = 0x02;
        }
        else
        {
            mSumCnt++;
        }
    }
};

#endif /* CAPSENS_H_ */

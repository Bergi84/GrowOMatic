#ifndef CAPSENS_H_
#define CAPSENS_H_

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "capSens.pio.h"
#include "hardware/irq.h"


template<uint sensPinCount>
class TCapSens {
    PIO pio;
    uint excPinBase;
    uint sensPinBase;
    uint sensPinPolMsk;

    uint sumBuf[4];
    uint sumCnt;
    uint sampleCnt;
    uint sumShift;
    uint aktivPin;

    uint maxChgCnt;

    inline void setSensCh(uint sm, uint gpio)
    {
        uint32_t tmp = pio->sm[sm].execctrl;
        tmp &= ~PIO_SM0_EXECCTRL_JMP_PIN_BITS;
        tmp |= gpio << PIO_SM0_EXECCTRL_JMP_PIN_LSB;
        pio->sm[sm].execctrl = tmp;
    }

    inline void setNextSensPins()
    {
        uint tmp = aktivPin;
        for(int i = 0; i < 4; i++)
        {
            setSensCh(i, tmp + sensPinBase);
            if(tmp == sensPinCount - 1)
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
    uint capVal[sensPinCount];
    void init(PIO aPio, uint aExcPinBase, uint aSensPinBase, uint aSensPinPolMsk, uint aMaxChgCnt, uint aDchgCnt)
    {
        pio = aPio;
        excPinBase = aExcPinBase;
        sensPinBase = aSensPinBase;
        sensPinPolMsk = aSensPinPolMsk;
        maxChgCnt = aMaxChgCnt;

        capSens_init(aPio, aExcPinBase, aSensPinBase, sensPinCount, aSensPinPolMsk, aMaxChgCnt, aDchgCnt);

        for(uint i = 0; i < sensPinCount; i++)
        {
            sumBuf[i] = 0;
        }
        setOverSampling(256);
        sumCnt = 0;
        aktivPin = 0;

        setNextSensPins();

        mDebEmptyCall = 0;
    }

    void enable()
    {
        pio_set_sm_mask_enabled (pio, 0xF, true); 
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
        sumShift = cnt;
        sampleCnt = 1 << cnt;
    }

    // this function installs the irq handler and only accepts "C" functions
    // or static memeber function. So we must provide an IRQ Handler wrapper function
    // for our capSens->irqHandler memeber function

    void setIrqHandler(void (*aIrqHandler)(void))
    {
        pio_set_irq0_source_enabled(pio, pis_interrupt0 , true);
        if(pio == pio0)
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
        pio_interrupt_clear(pio, 0);

        // for debugging, should never occure
        // rx fifo has no Values

        if((pio->fstat & 0x00000F00) != 0)
        {
            mDebEmptyCall++;
            return;
        }

        for(int i = 0; i < 4; i++)
        {
            sumBuf[i] += pio->rxf[i];
        }

        if(sumCnt == sampleCnt - 1)
        {
            // stop aquisition
            pio->irq_force = 0x02;
            while(pio->sm[0].addr != 20); // should be the wait instruction of master (wait 0 irq 1)

            for(int i = 0; i < 4; i++)
            {
                capVal[aktivPin] = maxChgCnt - (sumBuf[i] >> sumShift);
                sumBuf[i] = 0;
                if(aktivPin == sensPinCount - 1)
                {
                    aktivPin = 0;
                }
                else
                {
                    aktivPin++;
                }
            }
            sumCnt = 0;

            // clear fifo
            while(!(pio->fstat & 0x00000100))
                volatile uint32_t dump = pio->rxf[0];
            while(!(pio->fstat & 0x00000200))
                volatile uint32_t dump = pio->rxf[1];
            while(!(pio->fstat & 0x00000400))
                volatile uint32_t dump = pio->rxf[2];
            while(!(pio->fstat & 0x00000800))
                volatile uint32_t dump = pio->rxf[3];      

            setNextSensPins();

            // start pio
            pio->irq = 0x02;
        }
        else
        {
            sumCnt++;
        }
    }
};

#endif /* CAPSENS_H_ */

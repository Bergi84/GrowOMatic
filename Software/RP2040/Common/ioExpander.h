#ifndef IOEXPANDER_H_
#define IOEXPANDER_H_

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "ioExpander.pio.h"

class TIoExpander {
private:
    PIO mPio;
    uint mSmNo;
    uint mOutState;

public:
    void init(PIO aPio, uint aSmNo, uint csGpio, uint dataGpio, uint bitCnt, uint aClkdDiv)
    {
        ioExpander_init(aPio, aSmNo, csGpio, dataGpio, bitCnt, aClkdDiv);
        mPio = aPio;
        mSmNo = aSmNo;
    }

    inline static void setIO(void* aArg, uint32_t aOut, uint32_t aMsk)
    {
        TIoExpander* pObj = (TIoExpander*) aArg;

        pObj->mOutState = (pObj->mOutState & ~aMsk) | (aOut & aMsk);
        pObj->mPio->txf[pObj->mSmNo] = pObj->mOutState;
    }

};

#endif
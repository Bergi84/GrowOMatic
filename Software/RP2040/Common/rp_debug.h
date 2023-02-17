#ifndef RP_DEBUG_H_
#define RP_DEBUG_H_

#include "stdint.h"
#include "pico/stdlib.h"

class TDebug
{
private:
    bool mInit;

    uint32_t mPinMap[32];

public:
    TDebug();

    void init(uint32_t pinMsk);

    inline void setPin(uint32_t pinNo)
    {
        if(mInit) gpio_set_mask(mPinMap[pinNo]);
    }

    inline void resetPin(uint32_t pinNo)
    {
        if(mInit) gpio_clr_mask(mPinMap[pinNo]);
    }

    inline void togglePnt(uint32_t pinNo)
    {
        if(mInit) gpio_xor_mask(mPinMap[pinNo]);
    }

    uint32_t mHighestPin;
};

extern TDebug gDebug;

#endif
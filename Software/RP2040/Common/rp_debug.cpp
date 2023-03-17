#include "rp_debug.h"

TDebug gDebug;

TDebug::TDebug()
{
    mInit = false;
}

void TDebug::init(uint32_t pinMsk)
{
    uint32_t mHighestPin = 0;
    for(int i = 0; i < 32; i++)
    {
        if(((1 << i) & pinMsk) != 0)
        {
            mPinMap[mHighestPin] = 1 << i;
            mHighestPin++;
            gpio_clr_mask(1 << i);
            gpio_set_function(i, GPIO_FUNC_SIO);
            gpio_set_dir(i, true);
        } 
        else
        {
            mPinMap[mHighestPin] = 0;
        }
    }
    mInit = true;
}
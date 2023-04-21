#include "rp_pwmCapture.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "rp_pwmCapture.pio.h"
#include "hardware/dma.h"


void TPwmCapture::init(PIO aPio, uint aSmNo, uint aGpio, uint32_t aMaxPeriod, TDmaIrqMng* aIrqMng)
{
    mPio = aPio;
    mSmNo = aSmNo;
    mMaxPeriod = aMaxPeriod;

    // todo: init dma and pio

}
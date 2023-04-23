#include "rp_pwmCapture.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "rp_pwmCapture.pio.h"
#include "hardware/dma.h"

void TPwmCapture::init(PIO aPio, uint aSmNo, uint aGpio, uint32_t aMaxPeriod, TDmaIrqMng* aIrqMng, TTimerServer* aTS)
{
    mPio = aPio;
    mSmNo = aSmNo;
    mMaxPeriod = aMaxPeriod;
    mTS = aTS;
    mGpio = aGpio;

    mTimoutTimer = mTS->getTimer(timeOutCb, this);

    mDmaDataCh = dma_claim_unused_channel(true);
    mDmaCtrlCh = dma_claim_unused_channel(true);

    dma_channel_config cCtrl = dma_channel_get_default_config(mDmaCtrlCh);
    channel_config_set_transfer_data_size(&cCtrl, DMA_SIZE_32);
    channel_config_set_read_increment(&cCtrl, false);
    channel_config_set_write_increment(&cCtrl, false);

    mDmaWriteAdr = (uint32_t) &mHighCnt;

    dma_channel_configure(
        mDmaCtrlCh,
        &cCtrl,
        &dma_hw->ch[mDmaDataCh].al2_write_addr_trig,
        &mDmaWriteAdr,
        1,
        false
    );

    dma_channel_config cData = dma_channel_get_default_config(mDmaDataCh);
    channel_config_set_transfer_data_size(&cData, DMA_SIZE_32);
    channel_config_set_read_increment(&cData, false);
    channel_config_set_write_increment(&cData, true);
    channel_config_set_dreq(&cData, pio_get_dreq(aPio, aSmNo, false));
    channel_config_set_chain_to(&cData, mDmaCtrlCh);

    dma_channel_configure(
        mDmaDataCh,
        &cData,
        NULL,
        &aPio->rxf[aSmNo],
        2,
        false
    );
    aIrqMng->setDmaHandler(mDmaDataCh, irqCb, this);

    pwmCaptureInit(aPio, aSmNo, aGpio);
    gpio_set_pulls(aGpio, false, false);

    dma_start_channel_mask(1u << mDmaCtrlCh);
    pio_set_sm_mask_enabled (mPio, 1 << aSmNo, true); 

    mTimoutTimer->setTimer(aMaxPeriod);
}

void TPwmCapture::irqCb(void* aArg)
{
    TPwmCapture* pObj = (TPwmCapture*) aArg;

    pObj->mTimoutTimer->setTimer(pObj->mMaxPeriod);

}

uint32_t TPwmCapture::timeOutCb(void *aArg)
{
    TPwmCapture* pObj = (TPwmCapture*) aArg;

    pObj->mLenCnt = 0;

    return 0;
}
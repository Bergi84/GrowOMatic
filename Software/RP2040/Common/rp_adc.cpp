#include "rp_adc.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/dma.h"

void TAdc::init(uint32_t aGpioMsk, TDmaIrqMng* aIrqMng)
{
    for(int i = 0; i < 4; i++)
    {
        mFilt[i] = 0;
        mResult[i] = 0;
        mFiltLen[i] = 6;
    }

    for(int i = 26; i < 30; i++)
    {
        if((aGpioMsk & (1 << i)) != 0)
        {
            adc_gpio_init(i);
        }
    }
    adc_init();
    adc_set_round_robin(0x0F);
    adc_select_input(0);
    adc_fifo_setup(true, true, 1, false, false);

    mDmaWriteAdr[0] = (uint32_t) &mDmaBuf[0][0];
    mDmaWriteAdr[1] = (uint32_t) &mDmaBuf[1][0];
    mBufInd = 0;

    mDataDmaCh = dma_claim_unused_channel(true);
    mConfDmaCh = dma_claim_unused_channel(true);

    dma_channel_config cConf = dma_channel_get_default_config(mConfDmaCh);
    channel_config_set_transfer_data_size(&cConf, DMA_SIZE_32);
    channel_config_set_read_increment(&cConf, true);
    channel_config_set_write_increment(&cConf, false);
    channel_config_set_ring(&cConf, false, 3);

    dma_channel_configure(
        mConfDmaCh,
        &cConf,
        &dma_hw->ch[mDataDmaCh].al2_write_addr_trig,
        &mDmaWriteAdr[0],
        1,
        false
    );

    dma_channel_config cData = dma_channel_get_default_config(mDataDmaCh);
    channel_config_set_transfer_data_size(&cData, DMA_SIZE_16);
    channel_config_set_read_increment(&cData, false);
    channel_config_set_write_increment(&cData, true);
    channel_config_set_dreq(&cData, DREQ_ADC);
    channel_config_set_chain_to(&cData, mConfDmaCh);

    dma_channel_configure(
        mDataDmaCh,
        &cData,
        NULL,
        &adc_hw->fifo,
        4,
        false
    );
    aIrqMng->setDmaHandler(mDataDmaCh, irqCb, this);
    dma_start_channel_mask(1u << mConfDmaCh);

    adc_run(true);
}

void TAdc::irqCb(void* aArg)
{
    TAdc* pObj = (TAdc*) aArg; 

    for(int i = 0; i < 4; i++)
    {
        pObj->mFilt[i] = ((pObj->mFilt[i] * ((1UL << pObj->mFiltLen[i]) - 1UL)) >> pObj->mFiltLen[i]) + (uint32_t)pObj->mDmaBuf[pObj->mBufInd][i];
        pObj->mResult[i] = pObj->mFilt[i] >> pObj->mFiltLen[i];
    }
}
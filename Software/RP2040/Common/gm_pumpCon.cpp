#include "gm_pumpCon.h"

#include "string.h"
#include <stdio.h>

gm_pumpCon::gm_pumpCon()
{

}

void gm_pumpCon::init(TParaTable *aPT, TTimerServer *aTS, TSequencer* aSeq_c1)
{
    mPT = aPT;
    mTS = aTS;

    gpio_set_mask(1 << gpio_pc_spiEn);
    gpio_set_function(gpio_pc_spiEn, GPIO_FUNC_SIO);
    gpio_set_dir(gpio_pc_spiEn, true);

    mIOE.init(pio0, 0, gpio_pc_spiCs, gpio_pc_spiD, 32, 13);

    gpio_clr_mask(1 << gpio_pc_spiEn);

    const char periPumpName[] = "periPump";

    for(int i = 0; i < PPC_MAX_STEPPER; i++)
    {    
        mStepper[i].init(aPT, aTS, CPeriPumpBaseRegAdr + i*16);
        mStepper[i].setOutCb(i*4, mIOE.setIO, &mIOE);
        snprintf(mStepper[i].getEpName(), EP_NAME_LEN + 1, "%s%i", periPumpName, i);
    }

    mIrqMng.init(DMA_IRQ_1 ,aSeq_c1);
    mAdc.init(
        (1UL << gpio_pc_iSens0) |
        (1UL << gpio_pc_iSens1) |
        (1UL << gpio_pc_leakSens),
        &mIrqMng);

    mDcMotor[0].init(aPT, CPumpBaseRegAdr, aTS);
    mDcMotor[1].init(aPT, CPumpBaseRegAdr+256, aTS);
    mDcMotor[0].setGpioPwmEn(gpio_pc_pwmP0, gpio_pc_enP0);
    mDcMotor[1].setGpioPwmEn(gpio_pc_pwmP1, gpio_pc_enP1);
    mDcMotor[0].setCurAdc(&mAdc, gpio_pc_iSens0 - TAdc::gpio_adc_ch0, 2000);
    mDcMotor[1].setCurAdc(&mAdc, gpio_pc_iSens1 - TAdc::gpio_adc_ch0, 2000);
    mDcMotor[0].setGpioCurLim(gpio_pc_iRefP0, 2000);
    mDcMotor[1].setGpioCurLim(gpio_pc_iRefP1, 2000);

    const char pumpName[] = "dcPump";
    snprintf(mDcMotor[0].getEpName(), EP_NAME_LEN + 1, "%s%i", pumpName, 0);
    snprintf(mDcMotor[1].getEpName(), EP_NAME_LEN + 1, "%s%i", pumpName, 1);
}
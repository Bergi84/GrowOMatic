#include "gm_pumpCon.h"

#include "string.h"
#include <stdio.h>

GM_pumpCon::GM_pumpCon()
{

}

void GM_pumpCon::init(TParaTable *aPT, TTimerServer *aTS_c1, TSequencer* aSeq_c1)
{
    mPT = aPT;
    mTS = aTS_c1;

    gpio_set_mask(1 << gpio_pc_spiEn);
    gpio_set_function(gpio_pc_spiEn, GPIO_FUNC_SIO);
    gpio_set_dir(gpio_pc_spiEn, true);

    mIOE.init(pio0, 0, gpio_pc_spiCs, gpio_pc_spiD, 32, 13);

    gpio_clr_mask(1 << gpio_pc_spiEn);

    const char periPumpName[] = "periPump";

    for(int i = 0; i < PPC_MAX_STEPPER; i++)
    {    
        mStepper[i].init(aPT, mTS, CDefaultBaseRegAdr + i*16);
        mStepper[i].setOutCb(i*4, mIOE.setIO, &mIOE);
        snprintf(mStepper[i].getEpName(), EP_NAME_LEN + 1, "%s%i", periPumpName, i);
    }

    mIrqMng.init(DMA_IRQ_1 ,aSeq_c1);
    mAdc.init(
        (1UL << gpio_pc_iSens0) |
        (1UL << gpio_pc_iSens1) |
        (1UL << gpio_pc_leakSens),
        &mIrqMng);

    mDcMotor[0].init(aPT, CDefaultBaseRegAdr + 256, mTS);
    mDcMotor[1].init(aPT, CDefaultBaseRegAdr + 512, mTS);
    mDcMotor[0].setGpioPwmEn(gpio_pc_pwmP0, gpio_pc_enP0);
    mDcMotor[1].setGpioPwmEn(gpio_pc_pwmP1, gpio_pc_enP1);
    mDcMotor[0].setCurAdc(&mAdc, gpio_pc_iSens0 - TAdc::gpio_adc_ch0, 2000);
    mDcMotor[1].setCurAdc(&mAdc, gpio_pc_iSens1 - TAdc::gpio_adc_ch0, 2000);
    mDcMotor[0].setGpioCurLim(gpio_pc_iRefP0, 2000);
    mDcMotor[1].setGpioCurLim(gpio_pc_iRefP1, 2000);

    const char pumpName[] = "dcPump";
    snprintf(mDcMotor[0].getEpName(), EP_NAME_LEN + 1, "%s%i", pumpName, 0);
    snprintf(mDcMotor[1].getEpName(), EP_NAME_LEN + 1, "%s%i", pumpName, 1);

    const char pulsName[] = "flowPuls";
    mPulsSensor[0].init(aPT, CDefaultBaseRegAdr + 768, mTS, &mIrqMng, gpio_pc_flowPuls0, pio0, 1);
    snprintf(mPulsSensor[0].getEpName(), EP_NAME_LEN + 1, "%s%i", pulsName, 0);
    mPulsSensor[1].init(aPT, CDefaultBaseRegAdr + 1024, mTS, &mIrqMng, gpio_pc_flowPuls1, pio0, 2);
    snprintf(mPulsSensor[1].getEpName(), EP_NAME_LEN + 1, "%s%i", pulsName, 1);
    mLeakSensor.init(aPT, CDefaultBaseRegAdr + 1280, &mAdc, gpio_pc_leakSens - TAdc::gpio_adc_ch0);

    mLedCon.init(aPT, CDefaultBaseRegAdr + 1536, gpio_pc_ledPwm);
}
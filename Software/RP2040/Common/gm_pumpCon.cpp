#include "gm_pumpCon.h"

gm_pumpCon::gm_pumpCon()
{

}

void gm_pumpCon::init(TParaTable *aPT, TTimerServer *aTS)
{
    mPT = aPT;
    mTS = aTS;

    gpio_set_mask(1 << gpio_pc_spiEn);
    gpio_set_function(gpio_pc_spiEn, GPIO_FUNC_SIO);
    gpio_set_dir(gpio_pc_spiEn, true);

    mIOE.init(pio0, 0, gpio_pc_spiCs, gpio_pc_spiD, 32, 13);

    gpio_clr_mask(1 << gpio_pc_spiEn);

    for(int i = 0; i < PPC_MAX_STEPPER; i++)
    {    
        mStepper[i].init(aPT, aTS, CPeriPumpBaseRegAdr + i*16, i);
        mStepper[i].setOutCb(i*4, mIOE.setIO, &mIOE);
    }
}
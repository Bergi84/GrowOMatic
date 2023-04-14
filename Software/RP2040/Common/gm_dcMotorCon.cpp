#include "gm_dcMotorCon.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include <string.h>
#include "math.h"

gm_dcMotorCon::gm_dcMotorCon() :
mPara( (TParaTable::paraRec_t[cParaListLength]) {
    [PARA_PWMDUTY] =    {.para = 0,     .pFAccessCb = paraPwmDutyCb,.cbArg = this,  .defs = &cParaList[PARA_PWMDUTY]},
    [PARA_MAXPWMDUTY] = {.para = 1000,  .pFAccessCb = 0,            .cbArg = 0,     .defs = &cParaList[PARA_MAXPWMDUTY]},
    [PARA_FRQU] =       {.para = 32,    .pFAccessCb = paraFreq,     .cbArg = this,  .defs = &cParaList[PARA_FRQU]},
    [PARA_PWMRAMP]  =   {.para = 1000,  .pFAccessCb = paraRampCb,   .cbArg = this,  .defs = &cParaList[PARA_PWMRAMP]},
    [PARA_CURRENT] =    {.para = 0,     .pFAccessCb = paraCurCb,    .cbArg = this,  .defs = &cParaList[PARA_CURRENT]},
    [PARA_CUR_LIM] =    {.para = 500,   .pFAccessCb = paraCurLimCb, .cbArg = this,  .defs = &cParaList[PARA_CUR_LIM]},
}),
mEp( (TParaTable::endpoint_t) {  
    { { 
        .baseInd = CPumpBaseRegAdr,
        .type = (uint16_t)EPT_DCMOTORCON    
    } }, 
    .length = cParaListLength, 
    .para = mPara,
    .next = 0,
    .typeName = cTypeName   
})
{
    strncpy(mEp.epName, cTypeName, sizeof(mEp.epName));
}

void gm_dcMotorCon::init(TParaTable* aPT, uint32_t aBaseRegAdr, TTimerServer* aTS)
{
    mPT = aPT;
    mTS = aTS;

    
    aRampTimer = aTS->getTimer(rampTimerCb, this);

    mEp.epId.baseInd = aBaseRegAdr;
    mPT->addEndpoint(&mEp);

    mPwmRampInc = ((((uint64_t)mPara[PARA_PWMRAMP].para) * ((uint64_t)mPeriod)) << 16)/1000000ULL;
}

void gm_dcMotorCon::setGpioPwmEn(uint32_t aGpioPwm, uint32_t aGpioEn)
{
    mPwmComp = 0;
    mPwmAkt = 0;

    gpio_set_function(aGpioPwm, GPIO_FUNC_SIO);
    gpio_set_function(aGpioEn, GPIO_FUNC_SIO);
    gpio_set_dir(aGpioEn, true);
    gpio_set_dir(aGpioPwm, true);
    gpio_put(aGpioEn, false);
    gpio_put(aGpioPwm, false);
    mGpioEn = aGpioEn;
    mGpioPwm = aGpioPwm;
    mUni = true;

    mPwmSlice = pwm_gpio_to_slice_num(aGpioPwm);
    mPwmCh1 = pwm_gpio_to_channel(aGpioPwm);

    uint32_t fsys = clock_get_hz(clk_sys);
    mPeriod = fsys/(mPara[PARA_FRQU].para * 1000);

    pwm_set_clkdiv_int_frac(mPwmSlice, 1, 0);
    pwm_set_wrap(mPwmSlice, mPeriod - 1);
    pwm_set_output_polarity(mPwmSlice, true, true);
    pwm_set_chan_level(mPwmSlice, mPwmCh1, 0);
    pwm_set_enabled(mPwmSlice, true);
}

void gm_dcMotorCon::setGpioPwm(uint32_t aGpioPwm1, uint32_t aGpioPwm2)
{
    mPwmComp = 0;
    mPwmAkt = 0;

    gpio_set_function(aGpioPwm1, GPIO_FUNC_PWM);
    gpio_set_function(aGpioPwm2, GPIO_FUNC_PWM);
    mUni = false;

    mPwmSlice = pwm_gpio_to_slice_num(aGpioPwm1);
    mPwmCh1 = pwm_gpio_to_channel(aGpioPwm1);
    mPwmCh2 = pwm_gpio_to_channel(aGpioPwm2);

    uint32_t fsys = clock_get_hz(clk_sys);
    mPeriod = fsys/(mPara[PARA_FRQU].para * 1000);

    pwm_set_clkdiv_int_frac(mPwmSlice, 1, 0);
    pwm_set_wrap(mPwmSlice, mPeriod - 1);
    pwm_set_output_polarity(mPwmSlice, true, true);
    pwm_set_chan_level(mPwmSlice, mPwmCh1, mPeriod);
    pwm_set_chan_level(mPwmSlice, mPwmCh2, mPeriod);
    pwm_set_enabled(mPwmSlice, true);
}

void gm_dcMotorCon::setCurAdc(TAdc* aAdc, uint32_t aCh, uint32_t aFullScale)
{
    mAdc = aAdc;
    mCurAdcCh = aCh;
    mCurAdcScale = aFullScale;

    mAdc->setFilterLen(aCh, 6);
}

void gm_dcMotorCon::setGpioCurLim(uint32_t aGpio, uint32_t aFullScale)
{
    gpio_set_function(aGpio, GPIO_FUNC_PWM);
    mCurLimSlice = pwm_gpio_to_slice_num(aGpio);
    mCurLimCh = pwm_gpio_to_channel(aGpio);

    pwm_set_clkdiv_int_frac(mCurLimSlice, 1, 0);
    pwm_set_wrap(mCurLimSlice, cCurLimPeriod - 1);
    mCurLimScale = (cCurLimPeriod << 16)/aFullScale;
    pwm_set_chan_level(mCurLimSlice, mCurLimCh, (mCurLimScale * mPara[PARA_CUR_LIM].para) >> 16);
    pwm_set_enabled(mCurLimSlice, true);
}

void gm_dcMotorCon::paraPwmDutyCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite)
{
    gm_dcMotorCon* pObj = (gm_dcMotorCon*) aCbArg;

    int32_t locDuty = (int32_t) aPParaRec->para;

    if(pObj->mUni)
    {
        if(locDuty < 0)
            locDuty = 0;
        if(locDuty > 1000)
            locDuty = 1000;
    }
    else
    {
        if(locDuty > 1000)
            locDuty = 1000;
        if(locDuty < -1000)
            locDuty = -1000;
    }
    aPParaRec->para = locDuty;

    pObj->mPwmComp = (locDuty * pObj->mPeriod)/1000;

    if(!pObj->aRampTimer->isAktive())
        pObj->aRampTimer->setTimer(1000);
}

void gm_dcMotorCon::paraRampCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite)
{
    gm_dcMotorCon* pObj = (gm_dcMotorCon*) aCbArg;

    pObj->mPwmRampInc = ((((uint64_t)aPParaRec->para) * ((uint64_t)pObj->mPeriod)) << 16)/1000000ULL;
}
void gm_dcMotorCon::paraCurCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite)
{
    gm_dcMotorCon* pObj = (gm_dcMotorCon*) aCbArg;

    aPParaRec->para = (((uint32_t)pObj->mAdc->getResult(pObj->mCurAdcCh)) * pObj->mCurAdcScale) >> 12;
}

void gm_dcMotorCon::paraCurLimCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite)
{
    gm_dcMotorCon* pObj = (gm_dcMotorCon*) aCbArg;

    pwm_set_chan_level(pObj->mCurLimSlice, pObj->mCurLimScale, (pObj->mCurLimScale * aPParaRec->para) >> 16);
}

void gm_dcMotorCon::paraFreq(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite)
{
    gm_dcMotorCon* pObj = (gm_dcMotorCon*) aCbArg;

    uint32_t fsys = clock_get_hz(clk_sys);
    pObj->mPeriod = fsys/(pObj->mPara[PARA_FRQU].para * 1000);

    pwm_set_wrap(pObj->mPwmSlice, pObj->mPeriod - 1);
}

uint32_t gm_dcMotorCon::rampTimerCb(void* aArg)
{
    gm_dcMotorCon* pObj = (gm_dcMotorCon*) aArg;

    if(pObj->mPwmAkt == 0)
    {
        if(pObj->mPwmComp == 0)
        {
            if(pObj->mUni)
            {
                gpio_set_function(pObj->mGpioPwm, GPIO_FUNC_SIO);
                gpio_put(pObj->mGpioEn, false);
            }
            else
            {
                pwm_set_chan_level(pObj->mPwmSlice, pObj->mPwmCh1, pObj->mPeriod);
                pwm_set_chan_level(pObj->mPwmSlice, pObj->mPwmCh2, pObj->mPeriod);                   
            }

            return 0;
        }
        else
        {
            if(pObj->mUni)
            {
                gpio_set_function(pObj->mGpioPwm, GPIO_FUNC_PWM);
                gpio_put(pObj->mGpioEn, true);
            }
        }
    }

    bool rampDone = false;

    if((pObj->mPwmComp << 16) > pObj->mPwmAkt)
    {
        pObj->mPwmAkt += pObj->mPwmRampInc;

        if((pObj->mPwmComp << 16) <= pObj->mPwmAkt)
        {
            pObj->mPwmAkt = pObj->mPwmComp << 16;
            rampDone = true;
        }
    }
    else
    {
        pObj->mPwmAkt -= pObj->mPwmRampInc;

        if((pObj->mPwmComp << 16) >= pObj->mPwmAkt)
        {
            pObj->mPwmAkt = pObj->mPwmComp << 16;
            rampDone = true;
        }
    }

    if(pObj->mUni)
    {
        pwm_set_chan_level(pObj->mPwmSlice, pObj->mPwmCh1, pObj->mPwmAkt >> 16);
    }
    else
    {
        if(pObj->mPwmAkt >= 0)
        {
            pwm_set_chan_level(pObj->mPwmSlice, pObj->mPwmCh1, pObj->mPwmAkt >> 16);
            pwm_set_chan_level(pObj->mPwmSlice, pObj->mPwmCh2, 0);        
        }
        else
        {
            pwm_set_chan_level(pObj->mPwmSlice, pObj->mPwmCh1, 0);
            pwm_set_chan_level(pObj->mPwmSlice, pObj->mPwmCh2, (-1*pObj->mPwmAkt) >> 16);           
        }
    }

    if(rampDone)
    {
        if(pObj->mPwmAkt == 0)
            return 100000;  // 100ms breaking before shutdown
        else
            return 0;
    }
    else
        return 1000;
    


}
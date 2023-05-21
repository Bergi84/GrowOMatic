#include "gm_ledCon.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include <string.h>

GM_ledCon::GM_ledCon():
mPara( (TParaTable::paraRec_t[cParaListLength]) {
    [PARA_DIM] =    {.para = 0,     .pFAccessCb = paraDimCb, .cbArg = this,  .defs = &cParaList[PARA_DIM]},
    [PARA_FREQ] =   {.para = 20,     .pFAccessCb = paraFreqCb, .cbArg = this,  .defs = &cParaList[PARA_FREQ]}
}),
mEp( (TParaTable::endpoint_t) {  
    { { 
        .baseInd = CDefaultBaseRegAdr,
        .type = (uint16_t)cType  
    } }, 
    .length = cParaListLength, 
    .para = mPara,
    .next = 0,
    .typeName = cTypeName   
})
{
    strncpy(mEp.epName, cTypeName, sizeof(mEp.epName));
}

void GM_ledCon::init(TParaTable* aPT, uint32_t aBaseRegAdr,uint32_t mGpio)
{
    mPT = aPT;
    mEp.epId.baseInd = aBaseRegAdr;

    gpio_set_function(mGpio, GPIO_FUNC_PWM);

    mPwmSlice = pwm_gpio_to_slice_num(mGpio);
    mPwmCh = pwm_gpio_to_channel(mGpio);

    uint32_t fsys = clock_get_hz(clk_sys);
    mPeriod = fsys/(mPara[PARA_FREQ].para * 1000);
    mScale = (mPeriod * (1 << 16))/1000;

    pwm_set_clkdiv_int_frac(mPwmSlice, 1, 0);
    pwm_set_wrap(mPwmSlice, mPeriod - 1);
    pwm_set_chan_level(mPwmSlice, mPwmCh, 0);
    pwm_set_enabled(mPwmSlice, true);

    mPT->addEndpoint(&mEp);
}

void GM_ledCon::paraDimCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite)
{
    GM_ledCon* pObj = (GM_ledCon*) aCbArg;

    pwm_set_chan_level(pObj->mPwmSlice, pObj->mPwmCh, (pObj->mScale * aPParaRec->para)>> 16);   
}

void GM_ledCon::paraFreqCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite)
{
    GM_ledCon* pObj = (GM_ledCon*) aCbArg;

    uint32_t fsys = clock_get_hz(clk_sys);
    pObj->mPeriod = fsys/(pObj->mPara[PARA_FREQ].para * 1000);
    pObj->mScale = (pObj->mPeriod * (1 << 16))/1000;

    pwm_set_wrap(pObj->mPwmSlice, pObj->mPeriod - 1);
}

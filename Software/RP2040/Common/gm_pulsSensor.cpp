#include "gm_pulsSensor.h"
#include "string.h"
#include "hardware/clocks.h"

GM_pulsSensor::GM_pulsSensor() :
mPara( (TParaTable::paraRec_t[cParaListLength]) {
    [PARA_PULSPERMIN] =    {.para = 0,     .pFAccessCb = paraPulsPerMin,    .cbArg = this,  .defs = &cParaList[PARA_PULSPERMIN]},
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

void GM_pulsSensor::init(TParaTable* aPT, uint32_t aBaseRegAdr, TTimerServer* aTS, TDmaIrqMng* aIrqMng, uint32_t aGpio, PIO aPio, uint32_t aSmNo)
{
    mPT = aPT;
    mEp.epId.baseInd = aBaseRegAdr;

    mPwmC.init(aPio, aSmNo, aGpio, 1000000, aIrqMng, aTS);

    mPT->addEndpoint(&mEp);
}

void GM_pulsSensor::paraPulsPerMin(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite)
{
    GM_pulsSensor* pObj = (GM_pulsSensor*) aCbArg;

    uint32_t fsys = clock_get_hz(clk_sys);
    uint32_t periodLen = 
    aPParaRec->para = (fsys*60)/pObj->mPwmC.getPeriod();
}
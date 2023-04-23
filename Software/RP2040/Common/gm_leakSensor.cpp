#include "gm_leakSensor.h"
#include "string.h"

GM_leakSensor::GM_leakSensor() :
mPara( (TParaTable::paraRec_t[cParaListLength]) {
    [PARA_RESISTENCE] =    {.para = 0,     .pFAccessCb = paraResSec,    .cbArg = this,  .defs = &cParaList[PARA_RESISTENCE]},
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

void GM_leakSensor::init(TParaTable* aPT, uint32_t aBaseRegAdr, TAdc* aAdc, uint32_t aCh)
{
    mPT = aPT;
    mEp.epId.baseInd = aBaseRegAdr;

    mAdc = aAdc;
    mCh = aCh;

    mPT->addEndpoint(&mEp);
}

void GM_leakSensor::paraResSec(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite)
{
    GM_leakSensor* pObj = (GM_leakSensor*) aCbArg;

    uint32_t adcVal = pObj->mAdc->getResult(pObj->mCh);

    if(adcVal == cAdcRes)
        aPParaRec->para = 0xffffffff;
    else
        aPParaRec->para = (cRpull * cGain * cAdcRes)/(cAdcRes - adcVal);
}
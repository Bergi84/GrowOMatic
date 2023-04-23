#include "gm_digitalIn.h"
#include "string.h"
#include "hardware/gpio.h"

GM_digitalIn::GM_digitalIn() :
mPara( (TParaTable::paraRec_t[cParaListLength]) {
    [PARA_IN] =    {.para = 0,     .pFAccessCb = paraIn,    .cbArg = this,  .defs = &cParaList[PARA_IN]},
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

void GM_digitalIn::init(TParaTable* aPT, uint32_t aBaseRegAdr, uint32_t* aInReg, uint32_t aInCh)
{
    // todo: implement
    mPT = aPT;
    mEp.epId.baseInd = aBaseRegAdr;

    mReg = aInReg;
    mChMsk = 1 << aInCh;

    mPT->addEndpoint(&mEp);
}

void GM_digitalIn::paraIn(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite)
{
    GM_digitalIn* pObj = (GM_digitalIn*) aCbArg;

    if(*pObj->mReg & pObj->mChMsk)
        aPParaRec->para = 1;
    else
        aPParaRec->para = 0;
}

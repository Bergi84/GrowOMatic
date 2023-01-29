#include "gm_bus.h"

GM_bus::GM_bus() :
// init bus parameter list
mBusPara( (TParaTable::paraRec_t[cParaListLength]) {
    [PARA_EPNAME0] =     {.pPara = (uint32_t*) &mBusEp.epName[0],    .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_EPNAME0]},
    [PARA_EPNAME1] =     {.pPara = (uint32_t*) &mBusEp.epName[4],    .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_EPNAME1]},
    [PARA_EPNAME2] =     {.pPara = (uint32_t*) &mBusEp.epName[8],    .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_EPNAME2]},
    [PARA_EPNAME3] =     {.pPara = (uint32_t*) &mBusEp.epName[12],   .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_EPNAME3]},
    [PARA_MASTEREN] =    {.para = 0,  .pFAccessCb = setBusModeCb, .cbArg = this, .defs = &cParaList[PARA_MASTEREN]},
    }),

// init bus endpoint
mBusEp( (TParaTable::endpoint_t) { 
    { { 
        .baseInd= CInvalidReg,
        .type = (uint16_t)EPT_BUS    
    } }, 
    .length = cParaListLength, 
    .para = mBusPara,
    .next = 0
})
{

}

void GM_bus::init(TUart** aUartList, uint32_t aListLen, TSequencer* aSeq, TParaTable* aParaTable)
{
    for(int i = 0; i < aListLen; i++)
    {
        mUartList[i] = aUartList[i];
    }
    mListLen = aListLen;
    mSeq = aSeq;
    mParaTable = aParaTable;

    aParaTable->addEndpoint(&mBusEp);
}

void GM_bus::setBusModeCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite)
{
    GM_bus* pObj = (GM_bus*) aCbArg;

    if(pObj->mBusPara[0].para)
    {
        if(pObj->mSlave.isInit())
            pObj->mSlave.deinit();

        if(!pObj->mMaster.isInit())
            pObj->mMaster.init(pObj->mUartList, pObj->mListLen, pObj->mSeq, pObj->mParaTable);
    }
    else
    {
        if(!pObj->mSlave.isInit())
            pObj->mSlave.init(pObj->mUartList[0], pObj->mUartList[1], pObj->mParaTable, pObj->mSeq);

        if(pObj->mMaster.isInit())
            pObj->mMaster.deinit();
    }
}
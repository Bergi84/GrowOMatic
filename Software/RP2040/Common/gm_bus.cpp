#include "gm_bus.h"

GM_bus::GM_bus() :
// init bus parameter list
mBusPara( (TParaTable::paraRec_t[mBusParaLen]) {
    [PARA_MASTEREN] = {.para = 0,  .pFAccessCb = setBusModeCb, .cbArg = this, .flags = TParaTable::PARA_FLAG_RW | TParaTable::PARA_FLAG_NV | TParaTable::PARA_FLAG_FW},
    }),

// init bus endpoint
mBusEp( (TParaTable::endpoint_t) { 
    { { 
        .startIndex = TParaTable::CInvalidRegAdr,
        .type = (uint16_t)EPT_BUS    
    } }, 
    .length = sizeof(mBusPara)/sizeof(TParaTable::paraRec_t), 
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
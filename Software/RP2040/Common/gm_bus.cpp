#include "gm_bus.h"
#include <string.h>

GM_bus::GM_bus() :
// init bus parameter list
mBusPara( (TParaTable::paraRec_t[cParaListLength]) {
    [PARA_MASTEREN] =    {.para = 0,                                 .pFAccessCb = setBusModeCb, .cbArg = this,    .defs = &cParaList[PARA_MASTEREN]},
    }),

// init bus endpoint
mBusEp( (TParaTable::endpoint_t) { 
    { { 
        .baseInd= CBusBaseRegAdr,
        .type = (uint16_t)EPT_BUS    
    } }, 
    .length = cParaListLength, 
    .para = mBusPara,
    .next = 0,
    .typeName = cTypeName
})
{
    strcpy(mBusEp.epName, mBusEp.typeName);
}

void GM_bus::init(TUart** aUartList, uint32_t aListLen, TSequencer* aSeq, TParaTable* aParaTable, bool masterEn)
{
    for(int i = 0; i < aListLen; i++)
    {
        mUartList[i] = aUartList[i];
    }
    mListLen = aListLen;
    mSeq = aSeq;
    mParaTable = aParaTable;

    aParaTable->addEndpoint(&mBusEp);
    setBusMode(masterEn);
}

void GM_bus::setBusModeCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite)
{
    GM_bus* pObj = (GM_bus*) aCbArg;

    pObj->setBusMode(pObj->mBusPara[PARA_MASTEREN].para != 0);
}

void GM_bus::setBusMode(bool masterEn)
{
    if(masterEn)
    {
        if(mSlave.isInit())
            mSlave.deinit();

        if(!mMaster.isInit())
            mMaster.init(mUartList, mListLen, mSeq, mParaTable);
    }
    else
    {
        if(mMaster.isInit())
            mMaster.deinit();

        if(!mSlave.isInit())
            mSlave.init(mUartList[0], mUartList[1], mParaTable, mSeq);
    }
}
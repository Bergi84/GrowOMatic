#include "gm_bus.h"
#include <string.h>

GM_bus::GM_bus() :
// init bus parameter list
mBusPara( (TParaTable::paraRec_t[cParaListLength]) {
    [PARA_MASTEREN] =           {.para = 0,                                     .pFAccessCb = setBusModeCb, .cbArg = this,    .defs = &cParaList[PARA_MASTEREN]},
    [PARA_BUS_CNT] =            {.pPara = &mMaster.mBusNo,                      .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_BUS_CNT]},
    [PARA_BUS0_ERRCNTCRC] =     {.pPara = &mMaster.mBusCoor[0].mErrWrongCrc,    .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_BUS0_ERRCNTCRC]},
    [PARA_BUS1_ERRCNTCRC] =     {.pPara = &mMaster.mBusCoor[1].mErrWrongCrc,    .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_BUS1_ERRCNTCRC]},
    [PARA_BUS2_ERRCNTCRC] =     {.pPara = &mMaster.mBusCoor[2].mErrWrongCrc,    .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_BUS2_ERRCNTCRC]},
    [PARA_BUS3_ERRCNTCRC] =     {.pPara = &mMaster.mBusCoor[3].mErrWrongCrc,    .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_BUS3_ERRCNTCRC]},
    [PARA_BUS4_ERRCNTCRC] =     {.pPara = &mMaster.mBusCoor[4].mErrWrongCrc,    .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_BUS4_ERRCNTCRC]},
    [PARA_BUS0_ERRCNTTIMEOUT] = {.pPara = &mMaster.mBusCoor[0].mErrTimeout,     .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_BUS0_ERRCNTTIMEOUT]},
    [PARA_BUS1_ERRCNTTIMEOUT] = {.pPara = &mMaster.mBusCoor[1].mErrTimeout,     .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_BUS1_ERRCNTTIMEOUT]},
    [PARA_BUS2_ERRCNTTIMEOUT] = {.pPara = &mMaster.mBusCoor[2].mErrTimeout,     .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_BUS2_ERRCNTTIMEOUT]},
    [PARA_BUS3_ERRCNTTIMEOUT] = {.pPara = &mMaster.mBusCoor[3].mErrTimeout,     .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_BUS3_ERRCNTTIMEOUT]},
    [PARA_BUS4_ERRCNTTIMEOUT] = {.pPara = &mMaster.mBusCoor[4].mErrTimeout,     .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_BUS4_ERRCNTTIMEOUT]},
    [PARA_BUS0_ERRCNTNOECHO] =  {.pPara = &mMaster.mBusCoor[0].mErrNoecho,      .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_BUS0_ERRCNTNOECHO]},
    [PARA_BUS1_ERRCNTNOECHO] =  {.pPara = &mMaster.mBusCoor[1].mErrNoecho,      .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_BUS1_ERRCNTNOECHO]},
    [PARA_BUS2_ERRCNTNOECHO] =  {.pPara = &mMaster.mBusCoor[2].mErrNoecho,      .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_BUS2_ERRCNTNOECHO]},
    [PARA_BUS3_ERRCNTNOECHO] =  {.pPara = &mMaster.mBusCoor[3].mErrNoecho,      .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_BUS3_ERRCNTNOECHO]},
    [PARA_BUS4_ERRCNTNOECHO] =  {.pPara = &mMaster.mBusCoor[4].mErrNoecho,      .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_BUS4_ERRCNTNOECHO]}
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

void GM_bus::init(TUart** aUartList, uint32_t aListLen, TSequencer* aSeq, TTimerServer* aTimerServer, TParaTable* aParaTable, bool masterEn)
{
    for(int i = 0; i < aListLen; i++)
    {
        mUartList[i] = aUartList[i];
    }
    mListLen = aListLen;
    mSeq = aSeq;
    mParaTable = aParaTable;
    mTimerServer = aTimerServer;

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
            mMaster.init(mUartList, mListLen, mSeq, mTimerServer, mParaTable);
    }
    else
    {
        if(mMaster.isInit())
            mMaster.deinit();

        if(!mSlave.isInit())
            mSlave.init(mUartList[0], mUartList[1], mParaTable, mSeq, mTimerServer);
    }
}
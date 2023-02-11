#include "gm_system.h"
#include "pico/bootrom.h"
#include "hardware/watchdog.h"
#include <string.h>

TSystem::TSystem() : 
// init system parameter list
mSysPara( (TParaTable::paraRec_t[cParaListLength]) {
    [PARA_UID] =          {.para = 0,                                       .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_UID]},
    [PARA_TYPE] =         {.para = 0,                                       .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_TYPE]},
    [PARA_FWVERSION] =    {.para = VER_COMBO,                               .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_FWVERSION]},
    [PARA_SAVE] =         {.para = 0,                                       .pFAccessCb = paraSaveCb,   .cbArg = this,    .defs = &cParaList[PARA_SAVE]},
    [PARA_START] =        {.para = 0,                                       .pFAccessCb = paraStartCb,  .cbArg = this,    .defs = &cParaList[PARA_START]},
    [PARA_DEVNAME0] =     {.pPara = (uint32_t*) &mDevName[0],               .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_DEVNAME0]},
    [PARA_DEVNAME1] =     {.pPara = (uint32_t*) &mDevName[4],               .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_DEVNAME1]},
    [PARA_DEVNAME2] =     {.pPara = (uint32_t*) &mDevName[8],               .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_DEVNAME2]},
    [PARA_DEVNAME3] =     {.pPara = (uint32_t*) &mDevName[12],              .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_DEVNAME3]}
    }),
// init system endpoint
mSysEndpoint( (TParaTable::endpoint_t) { 
    { { 
        .baseInd = CSystemBaseRegAdr,
        .type = (uint16_t)EPT_SYSTEM    
    } }, 
    .length = cParaListLength, 
    .para = mSysPara,
    .next = 0,
    .typeName = cTypeName
})
{
    strcpy(mSysEndpoint.epName, mSysEndpoint.typeName);
}

void TSystem::init(uint32_t aUniqueId, devType_t aDevType, TParaTable* aParaTable)
{
    while(CInvalidUid == aUniqueId);

    mSysPara[PARA_UID].para = aUniqueId;
    mSysPara[PARA_TYPE].para = (uint32_t) aDevType;

    strcpy(mDevName, cDevTypeName[aDevType]);

    mPT = aParaTable;
    aParaTable->addEndpoint(&mSysEndpoint);
}

void TSystem::paraSaveCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite)
{
    TSystem* pObj = (TSystem*) aCbArg;

    switch(aPParaRec->para)
    {
        case 1:
            pObj->mPT->storePara();
            break;
        
        case 2:
            pObj->mPT->loadPara();
            break;

        defualt:
            pObj->mPT->clearPara();
            break;
    }
}

void TSystem::paraStartCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite)
{

    TParaTable* pObj = (TParaTable*) aCbArg;

    switch(aPParaRec->para)
    {
        case 1:
            // todo: remove hardware dependend code
            watchdog_enable(1, 1);
            while(1);
            break;
        
        case 2:
            // todo: remove hardware dependend code
            reset_usb_boot(0, 0);
            break;

        defualt:
            break;
    }
}
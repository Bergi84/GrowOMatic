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
    [PARA_FLASHLED] =     {.para = 0,                                       .pFAccessCb = paraFlashLed, .cbArg = this,    .defs = &cParaList[PARA_FLASHLED]},
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
    strncpy(mSysEndpoint.epName, mSysEndpoint.typeName, sizeof(mSysEndpoint.epName));
    mSysLed = 0;
    mSysLedTimerId = -1;
}

void TSystem::init(uint32_t aUniqueId, devType_t aDevType, TParaTable* aParaTable)
{
    while(CInvalidUid == aUniqueId);

    mSysPara[PARA_UID].para = aUniqueId;
    mSysPara[PARA_TYPE].para = (uint32_t) aDevType;

    strncpy(mDevName, cDevTypeName[aDevType], sizeof(mDevName));

    mPT = aParaTable;
    mPT->addEndpoint(&mSysEndpoint);
    mPT->setActiveCb(sysLedSignalAktivty, this);
}

void TSystem::setSysLed(uint32_t aGpioNo)
{
    mSysLed = 1 << aGpioNo;
    gpio_set_function(aGpioNo, GPIO_FUNC_SIO);
    gpio_set_dir(aGpioNo, true);
    gpio_drive_strength(GPIO_DRIVE_STRENGTH_12MA);

    gpio_set_mask(mSysLed);
}

void TSystem::sysLedFastFlash(uint32_t aSeconds)
{
    mFastFlashCnt = aSeconds * (1000000U / CFlashTimeUs) - 1;
    if(mSysLedTimerId == -1)
        mSysLedTimerId = add_alarm_in_us(CFlashTimeUs, sysLedCb, this, true);
}

void TSystem::sysLedSignalAktivty(void* aPObj)
{
    TSystem* pObj = (TSystem*) aPObj;

    if(pObj != 0 && pObj->mSysLedTimerId == -1)
    {
        pObj->mFastFlashCnt = 1;
        pObj->mSysLedTimerId = add_alarm_in_us(pObj->CFlashTimeUs, sysLedCb, aPObj, true);
    }
}

int64_t TSystem::sysLedCb(alarm_id_t id, void* aPObj)
{
    TSystem* pObj = (TSystem*) aPObj;

    if( (pObj->mFastFlashCnt & 0x0000000F) != 0)
        gpio_clr_mask(pObj->mSysLed);
    else
        gpio_set_mask(pObj->mSysLed);

    if(pObj->mFastFlashCnt > 0)
    {
        pObj->mFastFlashCnt--;
        return (int64_t)pObj->CFlashTimeUs * -1LL;
    }
    else
    {
        pObj->mSysLedTimerId = -1;
        return 0;
    }
}

void TSystem::paraSaveCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite)
{
    TSystem* pObj = (TSystem*) aCbArg;

    switch(aPParaRec->para)
    {
        case 0:
            break;

        case 1:
            pObj->mPT->storePara();
            break;
        
        case 2:
            pObj->mPT->loadPara();
            break;

        default:
            pObj->mPT->clearPara();
            break;
    }
}

void TSystem::paraStartCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite)
{
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

void TSystem::paraFlashLed(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite)
{
    TSystem* pObj = (TSystem*) aCbArg;

    pObj->sysLedFastFlash(aPParaRec->para);
}
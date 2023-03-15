#include "gm_system.h"
#include "pico/bootrom.h"
#include "hardware/watchdog.h"
#include "hardware/structs/watchdog.h"
#include "hardware/structs/psm.h"
#include "pico/multicore.h"
#include "uniFw.h"
#include <string.h>

TSystem::TSystem() : 
// init system parameter list
mSysPara( (TParaTable::paraRec_t[cParaListLength]) {
    [PARA_UID] =          {.para = 0,                                       .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_UID]},
    [PARA_TYPE] =         {.para = DT_INVALID,                              .pFAccessCb = paraTypeCb,   .cbArg = this,    .defs = &cParaList[PARA_TYPE]},
    [PARA_FWVERSION] =    {.para = VER_COMBO,                               .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_FWVERSION]},
    [PARA_SAVE] =         {.para = 0,                                       .pFAccessCb = paraSaveCb,   .cbArg = this,    .defs = &cParaList[PARA_SAVE]},
    [PARA_START] =        {.para = 0,                                       .pFAccessCb = paraStartCb,  .cbArg = this,    .defs = &cParaList[PARA_START]},
    [PARA_FLASHLED] =     {.para = 0,                                       .pFAccessCb = paraFlashLed, .cbArg = this,    .defs = &cParaList[PARA_FLASHLED]},
    [PARA_FWLEN] =        {.para = 0,                                       .pFAccessCb = paraFwLenCb,  .cbArg = this,    .defs = &cParaList[PARA_FWLEN]},
    [PARA_FWDATA] =       {.para = 0,                                       .pFAccessCb = paraFwDataCb, .cbArg = this,    .defs = &cParaList[PARA_FWDATA]},
    [PARA_FWCRC] =        {.para = 0,                                       .pFAccessCb = paraFwCrc,    .cbArg = this,    .defs = &cParaList[PARA_FWCRC]},
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

void TSystem::init(uint32_t aUniqueId, TParaTable* aParaTable)
{
    while(CInvalidUid == aUniqueId);

    mSysPara[PARA_UID].para = aUniqueId;

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

        case 0xdeadbeef:
            // todo: clear device type storage
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
            sysReset();
            break;
        
        case 2:
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

void TSystem::paraTypeCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite)
{
    TSystem* pObj = (TSystem*) aCbArg;

    if(pObj->mDevType == DT_INVALID)
    {  
        pObj->mDevType = (devType_t) aPParaRec->para;
        strncpy(pObj->mDevName, cDevTypeName[aPParaRec->para], sizeof(mDevName));
    }
    else
    {
        aPParaRec->para = pObj->mDevType;
    }
}

void TSystem::setDevType(devType_t aDevType)
{
    if(mDevType == DT_INVALID)
    {
        mDevType = aDevType;
        strncpy(mDevName, cDevTypeName[aDevType], sizeof(mDevName));
    }
}

void TSystem::paraFwLenCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite)
{
    TSystem* pObj = (TSystem*) aCbArg;

    uint32_t eraseSize = (((aPParaRec->para << 2) + FLASH_SECTOR_SIZE - 1)/FLASH_SECTOR_SIZE)*FLASH_SECTOR_SIZE;

    if(eraseSize > FLASH_FW_BUFFER_SIZE)
    {
        pObj->mFwLen = 0;
        pObj->mFwCrc = -1;
        return;
    }

    pObj->mFwLen = aPParaRec->para;
    pObj->mFwDataBufInd = 0;
    pObj->mFwFlashOff = 0;
    pObj->mFwCrc = 0;
}

void TSystem::paraFwDataCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite)
{
    TSystem* pObj = (TSystem*) aCbArg;

    if(pObj->mFwDataBufInd == 0 && pObj->mFwFlashOff == 0 && pObj->mFwLen > 0)
    {
        uint32_t eraseSize = (((pObj->mFwLen << 2) + FLASH_SECTOR_SIZE - 1)/FLASH_SECTOR_SIZE)*FLASH_SECTOR_SIZE;
        TFlash::eraseSektors(FLASH_FW_BUFFER, eraseSize);
    }

    if(pObj->mFwDataBufInd < (FLASH_PAGE_SIZE >> 2) && pObj->mFwLen > 0)
    {
        pObj->mFwDataBuf[pObj->mFwDataBufInd] = aPParaRec->para;
        pObj->mFwCrc = TFlash::crcCalc(pObj->mFwCrc, aPParaRec->para);
    }
    pObj->mFwDataBufInd++;
}

void TSystem::paraFwCrc(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite)
{
    TSystem* pObj = (TSystem*) aCbArg;

    bool lastPage = (pObj->mFwDataBufInd + pObj->mFwFlashOff) == pObj->mFwLen;

    if((pObj->mFwDataBufInd == (FLASH_PAGE_SIZE >> 2) || lastPage) && pObj->mFwLen > 0)
    {
        pObj->mFwDataBufInd = 0;
        if(pObj->mFwCrc == aPParaRec->para)
        {
            TFlash::storePage(FLASH_FW_BUFFER + (pObj->mFwFlashOff << 2), (uint8_t*) pObj->mFwDataBuf);
            pObj->mFwFlashOff += (FLASH_PAGE_SIZE >> 2);

            if(lastPage)
            {
                // update firmware
                // first step: stop second core and disable all interrupts
                uint32_t status = save_and_disable_interrupts();
                // bug in SDK 1.4.0
                // multicore_lockout_start_blocking();
                multicore_lockout_start_timeout_us((uint64_t)356*24*60*60*1000*1000);

                // second step: erase firmware
                // after this step no acces to flash is allowed
                uint32_t eraseSize = (((pObj->mFwLen << 2) + FLASH_SECTOR_SIZE - 1)/FLASH_SECTOR_SIZE)*FLASH_SECTOR_SIZE;
                flash_range_erase(0, eraseSize);

                // third step: copy page wise the firmware from buffer to firmware location
                uint32_t cpyPos = 0;
                while(cpyPos < pObj->mFwFlashOff)
                {
                    uint32_t* buffer = (uint32_t*) (FLASH_FW_BUFFER + (cpyPos << 2));
                    for(int i = 0; i < (FLASH_PAGE_SIZE >> 2); i++)
                    {
                        pObj->mFwDataBuf[i] = *buffer;
                        buffer++;
                    }
                    flash_range_program(cpyPos << 2, (uint8_t*) pObj->mFwDataBuf, FLASH_PAGE_SIZE);
                    cpyPos += (FLASH_PAGE_SIZE >> 2);
                }

                // fourth step reboot
                sysReset();
            }
            return;
        }
        else
        {
            pObj->mFwLen = 0;
            pObj->mFwCrc = -2;  
        }
    }
    else
    {
        pObj->mFwLen = 0;
        pObj->mFwCrc = -1;  
    }
}

void TSystem::sysReset()
{
    // disable watchdog
    watchdog_hw->ctrl &= ~(WATCHDOG_CTRL_ENABLE_BITS | WATCHDOG_CTRL_PAUSE_DBG0_BITS |
        WATCHDOG_CTRL_PAUSE_DBG1_BITS | WATCHDOG_CTRL_PAUSE_JTAG_BITS);

    // configure to reset everything apart from oscillator
    psm_hw->wdsel = PSM_WDSEL_BITS & ~(PSM_WDSEL_ROSC_BITS | PSM_WDSEL_XOSC_BITS);

    // Trigger watchdog
    watchdog_hw->ctrl |= WATCHDOG_CTRL_TRIGGER_BITS;

    while(1);
}
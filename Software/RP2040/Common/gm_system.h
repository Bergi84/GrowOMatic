#ifndef GM_SYSTEM_H_
#define GM_SYSTEM_H_

#include "stdint.h"
#include "version.h"
#include "gm_busDefs.h"
#include "paraTable.h"
#include "rp_flash.h"

#include "pico/platform.h"
#include "pico/stdlib.h"

class TSystem : private TEpSysDefs
{
public:
    TSystem();

    void init(uint32_t aUniqueId, TParaTable* aParaTable);
    void setSysLed(uint32_t aGpioNo);
    void sysLedFastFlash(uint32_t aSeconds);
    void setDevType(devType_t aDevType);

private:
    static constexpr uint32_t CFlashTimeUs = 100000; 

    TParaTable::paraRec_t mSysPara[cParaListLength];
    TParaTable::endpoint_t mSysEndpoint;
    TParaTable* mPT;

    char mDevName[DEVICE_NAME_LEN + 1];

    uint32_t mSysLed;
    alarm_id_t mSysLedTimerId;
    uint32_t mFastFlashCnt;
    devType_t mDevType;

    uint32_t mFwLen;                                // fw len in words (aka uint32_t)
    uint32_t mFwDataBuf[FLASH_PAGE_SIZE>>2];
    uint32_t mFwDataBufInd;
    uint32_t mFwFlashOff;                           // flash write offset in words (aka uint32_t)
    uint32_t mFwCrc;

    static void paraSaveCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite);
    static void paraStartCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite);
    static void paraFlashLed(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite);
    static void paraTypeCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite);
    static void paraFwLenCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite);
    static void paraFwDataCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite);
    static void __not_in_flash_func(paraFwCrc)(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite);

    static void sysLedSignalAktivty(void* aPObj);
    static int64_t sysLedCb(alarm_id_t id, void* aPObj);

    static void __not_in_flash_func(sysReset)();
};

#endif 
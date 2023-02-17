#ifndef GM_SYSTEM_H_
#define GM_SYSTEM_H_

#include "stdint.h"
#include "version.h"
#include "gm_busDefs.h"
#include "paraTable.h"

#include "pico/platform.h"
#include "pico/stdlib.h"

class TSystem : private TEpSysDefs
{
public:
    TSystem();

    void init(uint32_t aUniqueId, devType_t aDevType, TParaTable* aParaTable);
    void setSysLed(uint32_t aGpioNo);

    void sysLedFastFlash(uint32_t aSeconds);

private:
    static constexpr uint32_t CFlashTimeUs = 100000; 

    TParaTable::paraRec_t mSysPara[cParaListLength];
    TParaTable::endpoint_t mSysEndpoint;
    TParaTable* mPT;

    char mDevName[DEVICE_NAME_LEN + 1];

    uint32_t mSysLed;
    alarm_id_t mSysLedTimerId;
    uint32_t mFastFlashCnt;

    static void paraSaveCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite);
    static void paraStartCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite);
    static void paraFlashLed(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite);

    static void sysLedSignalAktivty(void* aPObj);
    static int64_t sysLedCb(alarm_id_t id, void* aPObj);
};

#endif 
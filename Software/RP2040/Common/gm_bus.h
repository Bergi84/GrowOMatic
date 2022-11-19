#ifndef GM_BUS_H_
#define GM_BUS_H_

#include <stdint.h>
#include "gm_epLib.h"

class GM_Bus
{
protected:
    static constexpr uint8_t mSdW = 0x68;
    static constexpr uint8_t mSdR = 0xA2;
    static constexpr uint32_t mBaudRate = 115200;
    static constexpr uint8_t mInvalidAdr = -1;
    static constexpr uint32_t mCrcPoly = 0xEDB88320;

    static bool mCrcTabInit;
    static uint32_t mCrcTab[256];

    uint32_t crcCalc(uint32_t aCrc, uint8_t aByte);
    void crcInitTab();
    GM_Bus();
};

static constexpr uint32_t mSystemBaseRegAdr = 0x0000;
static constexpr uint32_t mEpListBaseRegAdr = 0x0010;

typedef enum {
    DT_INVALID,
    DT_DUAL_LEVEL_SENSOR = 1,
    DT_DUAL_VALVE_CON = 2,
} devType_t;

class GM_busMaster;

class TDevRec {
private:
    TDevRec();
    static const char* mDevNameList[];
    GM_busMaster* mBusMaster;
    uint32_t mUid;
    uint32_t mBusIndex;
    devType_t mType;

    uint32_t mEpScanInd;
    class TEpBase* epList;
    
    static void reqEpListLenCb (void* aArg, uint32_t* aVal);
    static void reqEp(void* aArg, uint32_t* aVal);

    bool mCon;
public: 
    TDevRec(uint32_t aUid, GM_busMaster* aBusMaster);

    const char* getDevName() {return mDevNameList[mType];}

    class TDevRec* next;
} ;


#endif /* GM_BUS_H_*/
#include "gm_bus.h"

bool GM_Bus::mCrcTabInit = false;
uint32_t GM_Bus::mCrcTab[256];

uint32_t GM_Bus::crcCalc(uint32_t aCrc, uint8_t aByte)
{
    return (aCrc >> 8) ^ mCrcTab[((uint8_t)aCrc) ^ aByte];
}

void GM_Bus::crcInitTab()
{
    for (uint32_t i = 0; i < 256; i++) {

        uint32_t crc = i;

        for (uint32_t j = 0; j < 8; j++) {

            if ( crc & 0x00000001 ) crc = ( crc >> 1 ) ^ GM_Bus::mCrcPoly;
            else                     crc =   crc >> 1;
        }

        GM_Bus::mCrcTab[i] = crc;
    }
}

GM_Bus::GM_Bus()
{
    if(!mCrcTabInit)
    {
        crcInitTab();
    }
}

const char* TDevRec::mDevNameList[] =
{
    [DT_INVALID] = "invalid",
    [DT_DUAL_LEVEL_SENSOR] = "dualLevelSensor",
    [DT_DUAL_VALVE_CON] = "dualValveCon"
};

TDevRec::TDevRec(uint32_t aUid, GM_busMaster* aBusMaster)
{
    mUid = aUid;
    GM_busMaster* mBusMaster = aBusMaster;
    devType_t mType = DT_INVALID;
    class TEpBase* epList = 0;
    mCon = true;
    mEpScanInd = 0;
}
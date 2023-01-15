#ifndef GM_BUSDEFS_H_
#define GM_BUSDEFS_H_

#include <stdint.h>
#include "crc32.h"

#define DEVICE_NAME_LEN     15
#define EP_NAME_LEN         15

typedef enum
{
    EC_SUCCESS,
    EC_INVALID_DEVADR,
    EC_INVALID_REGADR,
    EC_QUEUE_FULL,
    EC_TIMEOUT,
    EC_INVALID_UID,
    EC_NOT_INIT
} errCode_T;

static constexpr uint8_t CInvalidAdr = -1;
static constexpr uint8_t CInvalidBus = -1;

static constexpr uint32_t CSystemBaseRegAdr = 0x0000;
static constexpr uint32_t CEpListBaseRegAdr = 0x0010;

class GM_BusDefs : public TCrc32
{
protected:
    static constexpr uint8_t mSdW = 0x68;
    static constexpr uint8_t mSdR = 0xA2;
    static constexpr uint32_t mBaudRate = 115200;

    GM_BusDefs();
};

typedef enum {
    DT_INVALID,
    DT_DUAL_LEVEL_SENSOR = 1,
    DT_DUAL_VALVE_CON = 2,
} devType_t;

typedef enum
{
    DS_NEW,
    DS_AVAILABLE,
    DS_LOST
} devStat_t;

typedef struct
{
    uint32_t uid;
    uint16_t regAdr;
    uint8_t devAdr;
    uint8_t aBus;
}   
reqAdr_t;

typedef enum {
    EPT_INVALID = 0,
    EPT_SYSTEM = 1,
    EPT_EPLIST = 2,
    EPT_BUS = 3
} epType_t; 

class TEpSysDefs
{
protected:
    typedef enum
    {
        PARA_UID = 0,
        PARA_TYPE = 1,
        PARA_FWVERSION = 2,
        PARA_SAVE = 3,
        PARA_START = 4
    } paraInd_t; 

    static constexpr epType_t cType = EPT_SYSTEM;
};

class TEpBusDefs
{
protected:
    typedef enum
    {
        PARA_MASTEREN = 0
    } paraInd_t; 

    static constexpr epType_t cType = EPT_BUS;
};

#endif /* GM_BUSDEFS_H_*/
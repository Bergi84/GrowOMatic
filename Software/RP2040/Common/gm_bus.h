#ifndef GM_BUS_H_
#define GM_BUS_H_

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
    EC_INVALID_UID
} errCode_T;

static constexpr uint8_t CInvalidAdr = -1;
static constexpr uint8_t CInvalidBus = -1;

static constexpr uint32_t CSystemBaseRegAdr = 0x0000;
static constexpr uint32_t CEpListBaseRegAdr = 0x0010;

class GM_Bus : public TCrc32
{
protected:
    static constexpr uint8_t mSdW = 0x68;
    static constexpr uint8_t mSdR = 0xA2;
    static constexpr uint32_t mBaudRate = 115200;

    GM_Bus();
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


#endif /* GM_BUS_H_*/
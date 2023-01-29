#ifndef GM_BUSDEFS_H_
#define GM_BUSDEFS_H_

#include <stdint.h>
#include "commonDefs.h"
#include "crc32.h"

#define DEVICE_NAME_LEN     15
#define EP_NAME_LEN         15



static constexpr uint32_t PARA_FLAG_W =  0x00000001;     // is writable
static constexpr uint32_t PARA_FLAG_R =  0x00000002;     // is readable
static constexpr uint32_t PARA_FLAG_RW = 0x00000003;     // is write and readable
static constexpr uint32_t PARA_FLAG_NV = 0x00000004;     // is non volatile stored
static constexpr uint32_t PARA_FLAG_S =  0x00000008;     // is scopable
static constexpr uint32_t PARA_FLAG_FR = 0x00000010;     // call update callback before read 
static constexpr uint32_t PARA_FLAG_FW = 0x00000020;     // call update callback after write
static constexpr uint32_t PARA_FLAG_P =  0x00000040;     // parameter is a pointer

typedef struct {
    uint32_t flags;
    const char* paraName;
} paraDef_t;

static constexpr uint8_t CInvalidAdr = -1;
static constexpr uint8_t CInvalidBus = -1;
static constexpr uint16_t CInvalidReg = -1;
static constexpr uint32_t CInvalidUid = -1;

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

constexpr const char* cDevTypeName[] = {
    [DT_INVALID] =              "unknowen",
    [DT_DUAL_LEVEL_SENSOR] =    "DualLevelSensor",
    [DT_DUAL_VALVE_CON] =       "DualValveSensor",
};

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

typedef union {
    struct {
        uint16_t baseInd;
        uint16_t type;
    };
    uint32_t id;
} epId_u;

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
        PARA_TYPE,
        PARA_FWVERSION,
        PARA_SAVE,
        PARA_START,
        PARA_DEVNAME0,
        PARA_DEVNAME1,
        PARA_DEVNAME2,
        PARA_DEVNAME3,
    } paraInd_t; 

    static constexpr epType_t cType = EPT_SYSTEM;

    static constexpr paraDef_t cParaList[] = {
        [PARA_UID] =        {PARA_FLAG_R,                   "uniqueId"},
        [PARA_TYPE] =       {PARA_FLAG_R,                   "deviceType"},
        [PARA_FWVERSION] =  {PARA_FLAG_R,                   "fwVersion"},
        [PARA_SAVE] =       {PARA_FLAG_W | PARA_FLAG_FW,    "savePara"},
        [PARA_START] =      {PARA_FLAG_W | PARA_FLAG_FW,    "start"},
        [PARA_DEVNAME0] =   {PARA_FLAG_RW | PARA_FLAG_NV | PARA_FLAG_P,    "devName0"},
        [PARA_DEVNAME1] =   {PARA_FLAG_RW | PARA_FLAG_NV | PARA_FLAG_P,    "devName1"},
        [PARA_DEVNAME2] =   {PARA_FLAG_RW | PARA_FLAG_NV | PARA_FLAG_P,    "devName2"},
        [PARA_DEVNAME3] =   {PARA_FLAG_RW | PARA_FLAG_NV | PARA_FLAG_P,    "devName3"},
    };
    static constexpr uint32_t cParaListLength = sizeof(cParaList)/ sizeof(paraDef_t); 
    static constexpr char cTypeName[] = "system";
};

class TEpBusDefs
{
protected:
    typedef enum
    {
        PARA_EPNAME0 = 0,
        PARA_EPNAME1,
        PARA_EPNAME2,
        PARA_EPNAME3,
        PARA_MASTEREN 
    } paraInd_t; 

    static constexpr epType_t cType = EPT_BUS;

    static constexpr paraDef_t cParaList[]  =
    {
        [PARA_EPNAME0] =        {PARA_FLAG_RW | PARA_FLAG_NV | PARA_FLAG_P,    "epName0"},
        [PARA_EPNAME1] =        {PARA_FLAG_RW | PARA_FLAG_NV | PARA_FLAG_P,    "epName1"},
        [PARA_EPNAME2] =        {PARA_FLAG_RW | PARA_FLAG_NV | PARA_FLAG_P,    "epName2"},
        [PARA_EPNAME3] =        {PARA_FLAG_RW | PARA_FLAG_NV | PARA_FLAG_P,    "epName3"},
        [PARA_MASTEREN] =       {PARA_FLAG_RW | PARA_FLAG_NV | PARA_FLAG_FW,   "masterEn"},
    };
    static constexpr uint32_t cParaListLength = sizeof(cParaList)/ sizeof(paraDef_t); 
    static constexpr char cTypeName[] = "bus";
};

#endif /* GM_BUSDEFS_H_*/
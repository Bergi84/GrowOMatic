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
static constexpr uint32_t PARA_FLAG_FRW = 0x00000030;     // call update callback after write
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
static constexpr uint32_t CBusBaseRegAdr = 0x0100;
static constexpr uint32_t CCapLevelBaseRegAdr = 0x0200;

static constexpr paraDef_t CEpNameDefs[] = {
    {PARA_FLAG_RW | PARA_FLAG_NV | PARA_FLAG_P, "epName0"},
    {PARA_FLAG_RW | PARA_FLAG_NV | PARA_FLAG_P , "epName1"},
    {PARA_FLAG_RW | PARA_FLAG_NV | PARA_FLAG_P , "epName2"},
    {PARA_FLAG_RW | PARA_FLAG_NV | PARA_FLAG_P , "epName3"},
};

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
    DT_PICO_BOARD = 3
} devType_t;

constexpr const char* cDevTypeName[] = {
    [DT_INVALID] =              "unknowen",
    [DT_DUAL_LEVEL_SENSOR] =    "DualLevelSensor",
    [DT_DUAL_VALVE_CON] =       "DualValveSensor",
    [DT_PICO_BOARD] =           "PicoBoard"
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
    EPT_BUS = 3,
    EPT_CAPLEVEL = 4
} epType_t; 

class TEpSysDefs
{
public:
    typedef enum
    {
        PARA_UID = 0,
        PARA_TYPE,
        PARA_FWVERSION,
        PARA_SAVE,
        PARA_START,
        PARA_FLASHLED,
        PARA_DEVNAME0,
        PARA_DEVNAME1,
        PARA_DEVNAME2,
        PARA_DEVNAME3,
    } paraInd_t; 

protected:
    static constexpr epType_t cType = EPT_SYSTEM;

    static constexpr paraDef_t cParaList[] = {
        [PARA_UID] =        {PARA_FLAG_R,                   "uniqueId"},
        [PARA_TYPE] =       {PARA_FLAG_R | PARA_FLAG_NV | PARA_FLAG_FW,    "deviceType"},
        [PARA_FWVERSION] =  {PARA_FLAG_R,                   "fwVersion"},
        [PARA_SAVE] =       {PARA_FLAG_W | PARA_FLAG_FW,    "savePara"},
        [PARA_START] =      {PARA_FLAG_W | PARA_FLAG_FW,    "start"},
        [PARA_FLASHLED] =   {PARA_FLAG_W | PARA_FLAG_FW,    "flashLed"},
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
        PARA_MASTEREN 
    } paraInd_t; 

    static constexpr epType_t cType = EPT_BUS;

    static constexpr paraDef_t cParaList[]  =
    {
        [PARA_MASTEREN] =       {PARA_FLAG_RW | PARA_FLAG_NV | PARA_FLAG_FW,   "masterEn"},
    };
    static constexpr uint32_t cParaListLength = sizeof(cParaList)/ sizeof(paraDef_t); 
    static constexpr char cTypeName[] = "bus";
};

class TEpCapLevelDefs
{
protected:
    typedef enum
    {
        PARA_LEVEL,
        PARA_VAL_AS,
        PARA_VAL_L10,
        PARA_VAL_L20,
        PARA_VAL_L30,
        PARA_VAL_L40,
        PARA_VAL_L50,
        PARA_VAL_L60,
        PARA_VAL_L70,
        PARA_VAL_L80,
        PARA_VAL_L90,
        PARA_VAL_L100,
        PARA_CAL,
        PARA_THRES,
        PARA_DIR,
        PARA_CAL_AS,
        PARA_CAL_L10,
        PARA_CAL_L20,
        PARA_CAL_L30,
        PARA_CAL_L40,
        PARA_CAL_L50,
        PARA_CAL_L60,
        PARA_CAL_L70,
        PARA_CAL_L80,
        PARA_CAL_L90,
        PARA_CAL_L100,
    } paraInd_t; 

    static constexpr epType_t cType = EPT_CAPLEVEL;

    static constexpr paraDef_t cParaList[]  =
    {
        [PARA_LEVEL] =       {PARA_FLAG_R | PARA_FLAG_FR,   "level"},
        [PARA_VAL_AS] =      {PARA_FLAG_R | PARA_FLAG_FR,   "valAS"},
        [PARA_VAL_L10] =     {PARA_FLAG_R | PARA_FLAG_FR,   "valL10"},
        [PARA_VAL_L20] =     {PARA_FLAG_R | PARA_FLAG_FR,   "valL20"},
        [PARA_VAL_L30] =     {PARA_FLAG_R | PARA_FLAG_FR,   "valL30"},
        [PARA_VAL_L40] =     {PARA_FLAG_R | PARA_FLAG_FR,   "valL40"},
        [PARA_VAL_L50] =     {PARA_FLAG_R | PARA_FLAG_FR,   "valL50"},
        [PARA_VAL_L60] =     {PARA_FLAG_R | PARA_FLAG_FR,   "valL60"},
        [PARA_VAL_L70] =     {PARA_FLAG_R | PARA_FLAG_FR,   "valL70"},
        [PARA_VAL_L80] =     {PARA_FLAG_R | PARA_FLAG_FR,   "valL80"},
        [PARA_VAL_L90] =     {PARA_FLAG_R | PARA_FLAG_FR,   "valL90"},
        [PARA_VAL_L100] =    {PARA_FLAG_R | PARA_FLAG_FR,   "valL100"},
        [PARA_CAL] =         {PARA_FLAG_W | PARA_FLAG_FW,   "cal"},
        [PARA_THRES] =       {PARA_FLAG_RW | PARA_FLAG_NV,  "threshold"},
        [PARA_DIR] =         {PARA_FLAG_RW | PARA_FLAG_NV,  "dir"},
        [PARA_CAL_AS] =      {PARA_FLAG_RW | PARA_FLAG_NV,   "calAS"},
        [PARA_CAL_L10] =     {PARA_FLAG_RW | PARA_FLAG_NV,   "calL10"},
        [PARA_CAL_L20] =     {PARA_FLAG_RW | PARA_FLAG_NV,   "calL20"},
        [PARA_CAL_L30] =     {PARA_FLAG_RW | PARA_FLAG_NV,   "calL30"},
        [PARA_CAL_L40] =     {PARA_FLAG_RW | PARA_FLAG_NV,   "calL40"},
        [PARA_CAL_L50] =     {PARA_FLAG_RW | PARA_FLAG_NV,   "calL50"},
        [PARA_CAL_L60] =     {PARA_FLAG_RW | PARA_FLAG_NV,   "calL60"},
        [PARA_CAL_L70] =     {PARA_FLAG_RW | PARA_FLAG_NV,   "calL70"},
        [PARA_CAL_L80] =     {PARA_FLAG_RW | PARA_FLAG_NV,   "calL80"},
        [PARA_CAL_L90] =     {PARA_FLAG_RW | PARA_FLAG_NV,   "calL90"},
        [PARA_CAL_L100] =     {PARA_FLAG_RW | PARA_FLAG_NV,   "calL100"}
    };
    static constexpr uint32_t cParaListLength = sizeof(cParaList)/ sizeof(paraDef_t); 
    static constexpr char cTypeName[] = "capLevel";
};

#endif /* GM_BUSDEFS_H_*/
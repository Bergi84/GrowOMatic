#ifndef GM_BUSDEFS_H_
#define GM_BUSDEFS_H_

#include <stdint.h>
#include "commonDefs.h"
#include "crc32.h"

#define DEVICE_NAME_LEN     15
#define EP_NAME_LEN         15

static constexpr uint32_t PARA_FLAG_W =  0x00000001;     // is writable
static constexpr uint32_t PARA_FLAG_R =  0x00000002;     // is readable
static constexpr uint32_t PARA_FLAG_NV = 0x00000004;     // is non volatile stored
static constexpr uint32_t PARA_FLAG_S =  0x00000008;     // is scopable
static constexpr uint32_t PARA_FLAG_FR = 0x00000010;     // call update callback before read 
static constexpr uint32_t PARA_FLAG_FW = 0x00000020;     // call update callback after write
static constexpr uint32_t PARA_FLAG_P =  0x00000040;     // parameter is a pointer

static constexpr uint32_t PARA_FLAG_RW = PARA_FLAG_R | PARA_FLAG_W;
static constexpr uint32_t PARA_FLAG_FRFW = PARA_FLAG_FR | PARA_FLAG_FW; 
static constexpr uint32_t PARA_FLAG_FRS = PARA_FLAG_FR | PARA_FLAG_S;


typedef struct {
    uint32_t flags;
    const char* paraName;
} paraDef_t;

static constexpr uint8_t CInvalidAdr = -1;
static constexpr uint8_t CBroadcastAdr = -2;
static constexpr uint8_t CInvalidBus = -1;
static constexpr uint16_t CInvalidReg = -1;
static constexpr uint32_t CInvalidUid = -1;

static constexpr uint32_t CSystemBaseRegAdr = 0x0000;
static constexpr uint32_t CEpListBaseRegAdr = 0x0020;
static constexpr uint32_t CBusBaseRegAdr = 0x0100;
static constexpr uint32_t CCapLevelBaseRegAdr = 0x0200;
static constexpr uint32_t CPeriPumpBaseRegAdr = 0x0200;
static constexpr uint32_t CPumpBaseRegAdr = 0x400;

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
    DT_PICO_BOARD = 3,
    DT_PUMP_CON = 4,
} devType_t;

constexpr const char* cDevTypeName[] = {
    [DT_INVALID] =              "unknowen",
    [DT_DUAL_LEVEL_SENSOR] =    "DualLevelSensor",
    [DT_DUAL_VALVE_CON] =       "DualValveSensor",
    [DT_PICO_BOARD] =           "PicoBoard",
    [DT_PUMP_CON] =             "PumpController"
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
    EPT_CAPLEVEL = 4,
    EPT_STEPPERCON = 5,
    EPT_DCMOTORCON = 6,
    EPT_LEAKSENSOR = 7,
    EPT_PULSSENSOR = 8,
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
        PARA_FWLEN,
        PARA_FWDATA,
        PARA_FWCRC,
        PARA_DEVNAME0,
        PARA_DEVNAME1,
        PARA_DEVNAME2,
        PARA_DEVNAME3,
    } paraInd_t; 

protected:
    static constexpr epType_t cType = EPT_SYSTEM;

    static constexpr paraDef_t cParaList[] = {
        [PARA_UID] =        {PARA_FLAG_R,                                  "uniqueId"},
        [PARA_TYPE] =       {PARA_FLAG_RW | PARA_FLAG_NV | PARA_FLAG_FW,   "deviceType"},
        [PARA_FWVERSION] =  {PARA_FLAG_R,                                  "fwVersion"},
        [PARA_SAVE] =       {PARA_FLAG_W | PARA_FLAG_FW,                   "savePara"},
        [PARA_START] =      {PARA_FLAG_W | PARA_FLAG_FW,                   "start"},
        [PARA_FLASHLED] =   {PARA_FLAG_W | PARA_FLAG_FW,                   "flashLed"},
        [PARA_FWLEN] =      {PARA_FLAG_W | PARA_FLAG_FW,                   "fwLen"},        // stores firmware length and starts firmware recieve process
        [PARA_FWDATA] =     {PARA_FLAG_W | PARA_FLAG_FW,                   "fwData"},       // recieves bit stream
        [PARA_FWCRC] =      {PARA_FLAG_W | PARA_FLAG_FW,                   "fwCrc"},        // writes data to flash, must written after every flash size with the aktuell crc value
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
        PARA_MASTEREN,
        PARA_BUS_CNT,
        PARA_BUS0_ERRCNTCRC,
        PARA_BUS1_ERRCNTCRC,
        PARA_BUS2_ERRCNTCRC,
        PARA_BUS3_ERRCNTCRC,
        PARA_BUS4_ERRCNTCRC,
        PARA_BUS0_ERRCNTTIMEOUT,
        PARA_BUS1_ERRCNTTIMEOUT,
        PARA_BUS2_ERRCNTTIMEOUT,
        PARA_BUS3_ERRCNTTIMEOUT,
        PARA_BUS4_ERRCNTTIMEOUT,
        PARA_BUS0_ERRCNTNOECHO,
        PARA_BUS1_ERRCNTNOECHO,
        PARA_BUS2_ERRCNTNOECHO,
        PARA_BUS3_ERRCNTNOECHO,
        PARA_BUS4_ERRCNTNOECHO,
    } paraInd_t; 

    static constexpr epType_t cType = EPT_BUS;

    static constexpr paraDef_t cParaList[]  =
    {
        [PARA_MASTEREN] =              {PARA_FLAG_RW | PARA_FLAG_NV | PARA_FLAG_FW,   "masterEn"},
        [PARA_BUS_CNT] =               {PARA_FLAG_R | PARA_FLAG_P,                    "busCnt"},
        [PARA_BUS0_ERRCNTCRC] =        {PARA_FLAG_RW | PARA_FLAG_P | PARA_FLAG_S,     "crcErr0"},
        [PARA_BUS1_ERRCNTCRC] =        {PARA_FLAG_RW | PARA_FLAG_P | PARA_FLAG_S,     "crcErr1"},
        [PARA_BUS2_ERRCNTCRC] =        {PARA_FLAG_RW | PARA_FLAG_P | PARA_FLAG_S,     "crcErr2"},
        [PARA_BUS3_ERRCNTCRC] =        {PARA_FLAG_RW | PARA_FLAG_P | PARA_FLAG_S,     "crcErr3"},
        [PARA_BUS4_ERRCNTCRC] =        {PARA_FLAG_RW | PARA_FLAG_P | PARA_FLAG_S,     "crcErr4"},
        [PARA_BUS0_ERRCNTTIMEOUT] =    {PARA_FLAG_RW | PARA_FLAG_P | PARA_FLAG_S,     "timeOut0"},
        [PARA_BUS1_ERRCNTTIMEOUT] =    {PARA_FLAG_RW | PARA_FLAG_P | PARA_FLAG_S,     "timeOut1"},
        [PARA_BUS2_ERRCNTTIMEOUT] =    {PARA_FLAG_RW | PARA_FLAG_P | PARA_FLAG_S,     "timeOut2"},
        [PARA_BUS3_ERRCNTTIMEOUT] =    {PARA_FLAG_RW | PARA_FLAG_P | PARA_FLAG_S,     "timeOut3"},
        [PARA_BUS4_ERRCNTTIMEOUT] =    {PARA_FLAG_RW | PARA_FLAG_P | PARA_FLAG_S,     "timeOut4"},
        [PARA_BUS0_ERRCNTNOECHO] =     {PARA_FLAG_RW | PARA_FLAG_P | PARA_FLAG_S,     "noEcho0"},
        [PARA_BUS1_ERRCNTNOECHO] =     {PARA_FLAG_RW | PARA_FLAG_P | PARA_FLAG_S,     "noEcho1"},
        [PARA_BUS2_ERRCNTNOECHO] =     {PARA_FLAG_RW | PARA_FLAG_P | PARA_FLAG_S,     "noEcho2"},
        [PARA_BUS3_ERRCNTNOECHO] =     {PARA_FLAG_RW | PARA_FLAG_P | PARA_FLAG_S,     "noEcho3"},
        [PARA_BUS4_ERRCNTNOECHO] =     {PARA_FLAG_RW | PARA_FLAG_P | PARA_FLAG_S,     "noEcho4"},
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
        [PARA_LEVEL] =       {PARA_FLAG_R | PARA_FLAG_FRS,   "level"},
        [PARA_VAL_AS] =      {PARA_FLAG_R | PARA_FLAG_FRS,   "valAS"},
        [PARA_VAL_L10] =     {PARA_FLAG_R | PARA_FLAG_FRS,   "valL10"},
        [PARA_VAL_L20] =     {PARA_FLAG_R | PARA_FLAG_FRS,   "valL20"},
        [PARA_VAL_L30] =     {PARA_FLAG_R | PARA_FLAG_FRS,   "valL30"},
        [PARA_VAL_L40] =     {PARA_FLAG_R | PARA_FLAG_FRS,   "valL40"},
        [PARA_VAL_L50] =     {PARA_FLAG_R | PARA_FLAG_FRS,   "valL50"},
        [PARA_VAL_L60] =     {PARA_FLAG_R | PARA_FLAG_FRS,   "valL60"},
        [PARA_VAL_L70] =     {PARA_FLAG_R | PARA_FLAG_FRS,   "valL70"},
        [PARA_VAL_L80] =     {PARA_FLAG_R | PARA_FLAG_FRS,   "valL80"},
        [PARA_VAL_L90] =     {PARA_FLAG_R | PARA_FLAG_FRS,   "valL90"},
        [PARA_VAL_L100] =    {PARA_FLAG_R | PARA_FLAG_FRS,   "valL100"},
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

class TEpStepperConDefs
{
protected:
    typedef enum
    {
        PARA_SPEED,
        PARA_POS,
        PARA_MAXSPEED,
        PARA_ACCEL,
        PARA_STEPS_PER_REV
    } paraInd_t; 

    static constexpr epType_t cType = EPT_STEPPERCON;

    static constexpr paraDef_t cParaList[]  =
    {
        [PARA_SPEED] =          {PARA_FLAG_RW | PARA_FLAG_FW,                  "speed"},              // milli Rev/s
        [PARA_POS] =            {PARA_FLAG_R | PARA_FLAG_S,                   "pos"},                // milli Rev
        [PARA_MAXSPEED] =       {PARA_FLAG_RW | PARA_FLAG_NV,                 "maxSpeed"},           // milli Rev/s
        [PARA_ACCEL] =          {PARA_FLAG_FW | PARA_FLAG_R | PARA_FLAG_NV,   "accel"},              // milli Rev/s^2
        [PARA_STEPS_PER_REV] =  {PARA_FLAG_RW | PARA_FLAG_NV,                 "stepsPeeRev"}         // steps/Rev

    };
    static constexpr uint32_t cParaListLength = sizeof(cParaList)/ sizeof(paraDef_t); 
    static constexpr char cTypeName[] = "stepper";   
};

class TEpDcMotorConDefs
{
protected:
    typedef enum
    {
        PARA_PWMDUTY,
        PARA_MAXPWMDUTY,
        PARA_FRQU,
        PARA_PWMRAMP,
        PARA_CURRENT,
        PARA_CUR_LIM
    } paraInd_t; 

    static constexpr epType_t cType = EPT_DCMOTORCON;

    static constexpr paraDef_t cParaList[]  =
    {
        [PARA_PWMDUTY] =      {PARA_FLAG_RW | PARA_FLAG_FW,                 "pwmDuty"},            // ‰
        [PARA_MAXPWMDUTY] =   {PARA_FLAG_RW | PARA_FLAG_NV,                 "pwmMaxDuty"},         // ‰
        [PARA_FRQU] =         {PARA_FLAG_RW | PARA_FLAG_FW | PARA_FLAG_NV,  "frquencie"},          // kHz
        [PARA_PWMRAMP] =      {PARA_FLAG_RW | PARA_FLAG_FW | PARA_FLAG_NV,  "ramp"},               // ‰/s
        [PARA_CURRENT] =      {PARA_FLAG_FRS | PARA_FLAG_R,                "current"},            // mA
        [PARA_CUR_LIM] =      {PARA_FLAG_RW | PARA_FLAG_FW |PARA_FLAG_NV,   "currLim"}            // mA
    };
    static constexpr uint32_t cParaListLength = sizeof(cParaList)/ sizeof(paraDef_t); 
    static constexpr char cTypeName[] = "dcMotor";   
};

class TEpLeakSensorDefs
{
    typedef enum
    {
        PARA_RESISTENCE
    } paraInd_t;  

    static constexpr epType_t cType = EPT_LEAKSENSOR;

    static constexpr paraDef_t cParaList[]  =
    {
        [PARA_RESISTENCE] =      {PARA_FLAG_FRS | PARA_FLAG_R,                "resistance"},            // Ohm
    };
    static constexpr uint32_t cParaListLength = sizeof(cParaList)/ sizeof(paraDef_t); 
    static constexpr char cTypeName[] = "leakSensor";       
};

class TEpPulsSensorDefs
{
    typedef enum
    {
        PARA_PULSPERSEC
    } paraInd_t;  

    static constexpr epType_t cType = EPT_PULSSENSOR;

    static constexpr paraDef_t cParaList[]  =
    {
        [PARA_PULSPERSEC] =      {PARA_FLAG_FRS | PARA_FLAG_R,                "pulsPerSec"},            
    };
    static constexpr uint32_t cParaListLength = sizeof(cParaList)/ sizeof(paraDef_t); 
    static constexpr char cTypeName[] = "pulsSensor";       
};

#endif /* GM_BUSDEFS_H_*/
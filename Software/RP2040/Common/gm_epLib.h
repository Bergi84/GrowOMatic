#ifndef GM_LIB_H_
#define GM_LIB_H_

#include <stdint.h>
#include "gm_busDefs.h"
#include "gm_device.h"

class GM_devUsedRec;

class TEpBase {
protected:
    friend class GM_device;

    TEpBase();

    class GM_device* mPDev;
    epType_t mType;
    uint16_t mBaseAdr;
    const paraDef_t* mParaList;
    uint32_t mParaListLen;
    const char* mTypeName;
    char mName[EP_NAME_LEN + 1];
    static constexpr char cInvalidName[] = "unkowen";

    void (*mNameCb) (void*, uint32_t*, errCode_T aStatus);
    void *mNameCbArg;
    uint8_t mNameInd;

    static void setEpNameCb(void*, uint32_t*, errCode_T aStatus);
    static void reqEpNameCb(void* aArg, uint32_t* aVal, errCode_T aStatus);

public:
    static TEpBase* newEp(epType_t aEpType, GM_device* aDev);
    virtual ~TEpBase();

    void regUsage(GM_devUsedRec* mDevUsedRec);
    void unregUsage(GM_devUsedRec* mDevUsedRec);

    inline epType_t getType() { return mType; };
    const char* getTypeName() {   return mTypeName;   };

    char* getEpName() {return mName;};
    errCode_T setEpName(char* aName, void (*aReqCb) (void*, uint32_t*, errCode_T), void* aArg);
    errCode_T reqEpName(void (*aReqCb) (void*, uint32_t*, errCode_T), void* aArg);

    uint32_t getParaPer(uint16_t aInd) {
        if(aInd < mParaListLen) 
            return mParaList[aInd].flags; 
        else if(aInd < mParaListLen + 4) 
            if(mType == EPT_SYSTEM)
                return CEpNameDefs[aInd - mParaListLen].flags & ~PARA_FLAG_W; 
            else
                return CEpNameDefs[aInd - mParaListLen].flags; 
        else 
            return 0;
    };

    const char* getParaName(uint16_t aInd) {
        if(aInd < mParaListLen) 
            return mParaList[aInd].paraName; 
        else if(aInd < mParaListLen + 4) 
            return CEpNameDefs[aInd - mParaListLen].paraName; 
        else 
            return 0;
    };

    uint32_t getParaListLen() {return mParaListLen + 4; };
    inline errCode_T setPara(uint16_t aIndOff, uint32_t aVal, void (*reqCb) (void*, uint32_t*, errCode_T aStatus), void* aArg, uint32_t aCooldDown = 0)
        { return mPDev->queueWriteReq(mBaseAdr + aIndOff, aVal, reqCb, aArg, aCooldDown); };
    inline errCode_T reqPara(uint16_t aIndOff, void (*reqCb) (void*, uint32_t*, errCode_T aStatus), void* aArg)
        { return mPDev->queueReadReq(mBaseAdr + aIndOff, reqCb, aArg); };

    uint16_t getBaseAdr() {return mBaseAdr;};

    TEpBase* mNext;
};

class TEpSystem : public TEpBase, private TEpSysDefs
{
private:
    TEpSystem();

    char* mDevName;
    static void setDevNameCb(void*, uint32_t*, errCode_T aStatus);
    static void reqDevNameCb(void* aArg, uint32_t* aVal, errCode_T aStatus);

public:
    TEpSystem(GM_device* aDev);

    // EP helper functions
    inline errCode_T getDevType(void (*reqEpListLenCb) (void*, uint32_t*, errCode_T aStatus), void* aArg )
    {   return mPDev->queueReadReq(CSystemBaseRegAdr + PARA_TYPE, reqEpListLenCb, aArg);  };

    errCode_T setDevName(char* aName, void (*aReqCb) (void*, uint32_t*, errCode_T), void* aArg);
    errCode_T reqDevName(char* aName, void (*aReqCb) (void*, uint32_t*, errCode_T), void* aArg);    
};

class TEpBus : public TEpBase, private TEpBusDefs
{
private:
    TEpBus();

public:
    TEpBus(GM_device* aDev);

    // EP helper functions
};

class TEpCapLevel : public TEpBase, private TEpCapLevelDefs
{
private:
    TEpCapLevel();

public:
    TEpCapLevel(GM_device* aDev);
    
    // EP helper functions
};

class TEpStepperCon : public TEpBase, private TEpStepperConDefs
{
private:
    TEpStepperCon();

public:
    TEpStepperCon(GM_device* aDev);
    
    // EP helper functions
};

class TEpDcMotorCon : public TEpBase, private TEpDcMotorConDefs
{
private:
    TEpDcMotorCon();

public:
    TEpDcMotorCon(GM_device* aDev);
    
    // EP helper functions
};

class TEpLeakSensor : public TEpBase, private TEpLeakSensorDefs
{
private:
    TEpLeakSensor();

public:
    TEpLeakSensor(GM_device* aDev);
    
    // EP helper functions
};

class TEpPulsSensor : public TEpBase, private TEpPulsSensorDefs
{
private:
    TEpPulsSensor();

public:
    TEpPulsSensor(GM_device* aDev);
    
    // EP helper functions
};

class TEpDigitalIn : public TEpBase, private TEpDigitalInDefs
{
private:
    TEpDigitalIn();

public:
    TEpDigitalIn(GM_device* aDev);
    
    // EP helper functions
};

#endif /*GM_LIB_H*/
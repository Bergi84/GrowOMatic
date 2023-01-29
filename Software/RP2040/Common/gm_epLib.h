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
    static const char cInvalidName[];

public:
    static TEpBase* newEp(epType_t aEpType);
    void regUsage(GM_devUsedRec* mDevUsedRec);
    void unregUsage(GM_devUsedRec* mDevUsedRec);
    inline epType_t getType() { return mType; };
    const char* getTypeName() {   return mTypeName;   };
    char* getEpName() {return mName;};
    void setEpName(char* aName);
    void reqEpName();

    const char* getParaName(uint16_t aInd) {if(aInd < mParaListLen) return mParaList[aInd].paraName; else return 0;};
    uint32_t getParaListLen() {return mParaListLen; };

    uint16_t getBaseAdr() {return mBaseAdr;};

    TEpBase* mNext;
};

class TEpSystem : public TEpBase, private TEpSysDefs
{
private:


public:
    TEpSystem();

    // EP helper functions
    inline void getDevType(void (*reqEpListLenCb) (void*, uint32_t*, errCode_T aStatus), void* aArg )
    {   mPDev->queueReadReq(0x0000 + PARA_TYPE, reqEpListLenCb, aArg);  };
};

class TEpBus : public TEpBase, private TEpBusDefs
{
private:

public:
    TEpBus();
    // EP helper functions
};

#endif /*GM_LIB_H*/
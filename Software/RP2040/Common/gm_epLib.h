#ifndef GM_LIB_H_
#define GM_LIB_H_

#include <stdint.h>
#include "gm_busDefs.h"
#include "gm_device.h"

typedef struct paraDef_s {
    static constexpr uint32_t PARA_FLAG_W =  0x00000001;     // is writable
    static constexpr uint32_t PARA_FLAG_R =  0x00000002;     // is readable
    static constexpr uint32_t PARA_FLAG_RW = 0x00000003;    // is write and readable

    const uint32_t flags;
    const char* paraName;
} paraDef_t;

class GM_devUsedRec;

class TEpBase {
protected:
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
    void generateName();

    TEpBase* mNext;
};


class TEpSystem : public TEpBase, private TEpSysDefs
{
private:
    static const paraDef_t cParaList[];
    static const uint32_t cParaListLength; 
    static const char cTypeName[];

public:
    TEpSystem();

    // EP helper functions
    inline void getDevType(void (*reqEpListLenCb) (void*, uint32_t*, errCode_T aStatus), void* aArg )
    {   mPDev->queueReadReq(0x0000 + PARA_TYPE, reqEpListLenCb, aArg);  };
};

class TEpBus : public TEpBase, private TEpBusDefs
{
private:
    static const paraDef_t cParaList[];
    static const uint32_t cParaListLength; 
    static const char cTypeName[];

public:
    TEpBus();
    // EP helper functions
};

#endif /*GM_LIB_H*/
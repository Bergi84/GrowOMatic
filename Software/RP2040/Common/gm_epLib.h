#ifndef GM_LIB_H_
#define GM_LIB_H_

#include <stdint.h>
#include "gm_bus.h"

typedef struct paraDef_s {
    static constexpr uint32_t PARA_FLAG_W =  0x00000001;     // is writable
    static constexpr uint32_t PARA_FLAG_R =  0x00000002;     // is readable
    static constexpr uint32_t PARA_FLAG_RW = 0x00000003;    // is write and readable

    const uint32_t flags;
    const char* paraName;
} paraDef_t;

typedef enum {
    EPT_INVALID = 0,
    EPT_SYSTEM = 1,
    EPT_EPLIST = 2,
} epType_t; 

class GM_device;
class GM_devUsedRec;

class TEpBase {
private:
    TEpBase();

protected:
    class GM_device* mPDev;
    epType_t mType;
    uint16_t mBaseAdr;
    const paraDef_t* mParaList;
    const uint32_t mParaListLen;
    const char* mEpName;

public:
    static TEpBase* newEp(epType_t aEpType);
    void regUsage(GM_devUsedRec* mDevUsedRec);
    void unregUsage(GM_devUsedRec* mDevUsedRec);
    TEpBase* mNext;
};


class TEpSystem : public TEpBase
{
private:
    typedef enum
    {
        PARA_UID = 0,
        PARA_TYPE = 1,
        PARA_FWVERSION = 2
    } paraInd_t; 

    static const paraDef_t cParaList[];
    static const uint32_t cParaListLength; 
    static const char cName[];
    static constexpr epType_t cType = EPT_SYSTEM;

public:
    TEpSystem();
    void getDevType(void (*reqEpListLenCb) (void*, uint32_t*, errCode_T aStatus), void* aArg );
};

#endif /*GM_LIB_H*/
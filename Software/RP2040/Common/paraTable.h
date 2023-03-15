#ifndef PARATABLE_H_
#define PARATABLE_H_

#include "storage.h"
#include "gm_busDefs.h"

#ifndef PT_MAXENDPOINTS
#define PT_MAXENDPOINTS 16
#endif

class TParaTable
{
public:
    TParaTable();

    typedef struct paraRec_s
    {
        union{
            uint32_t para;
            uint32_t* pPara;
        };
        void (*pFAccessCb)(void* aCbArg, struct paraRec_s* aPParaRec, bool aWrite);
        void *cbArg;
        const paraDef_t* defs;
    } paraRec_t;

    typedef struct endpoint_s
    {
        epId_u epId;
        uint32_t length;
        paraRec_t* para;
        struct endpoint_s* next;
        const char* typeName;
        char epName[EP_NAME_LEN + 1];
    } endpoint_t;

    void init(TStorage* aStorage);
    void addEndpoint(endpoint_t* aEndpoint_t);
    endpoint_t* findEp(uint16_t baseInd);
    endpoint_t* getEpLL() {return mRootEp;};
    errCode_T __time_critical_func(setPara)(uint16_t aRegAdr, uint32_t aData);
    errCode_T __time_critical_func(getPara)(uint16_t aRegAdr, uint32_t *aData);
    errCode_T getParaName(uint16_t aRegAdr, const char **aData);
    errCode_T getParaPer(uint16_t aRegAdr, uint32_t *aData);
    bool getParaAdr(uint16_t aRegAdr, uint32_t** aPraRec);
    void setActiveCb(void (*aCb)(void* aArg), void* aArg);

    void loadPara();
    void storePara();
    void clearPara();

    void loadDefault(uint16_t aRegAdr = CInvalidReg);

private:
    TStorage* mStorage;

    paraRec_t* __time_critical_func(findPara)(uint16_t index);

    endpoint_t* mRootEp;

    paraRec_t mEpListPara[PT_MAXENDPOINTS];
    endpoint_t mEpListEndpoint;
    static constexpr paraDef_t mEpListEndpointDefs = {PARA_FLAG_R, 0};

    TParaTable::paraRec_t mTmpPara;

    endpoint_t* mStoreLoadEp;
    uint16_t mStoreLoadInd;
    uint16_t mStoreLoadCnt;
    bool mStoreLoadDone;
    uint32_t mNVCheckSum;
    uint32_t mNVLen;
    void (*mActiveCb)(void* aArg);
    void* mActiveCbArg;

    void calcNVCheckSum(uint32_t* aCheckSum, uint32_t* aNVParaCnt);
    static bool storeParaCb(void* aArg, uint32_t* aData, uint32_t aLen);
    static bool loadParaCb(void* aArg, uint32_t* aData, uint32_t aLen);
};

#endif /* PARATABLE_H_ */
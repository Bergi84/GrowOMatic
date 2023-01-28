#ifndef PARATABLE_H_
#define PARATABLE_H_

#include "stdint.h"
#include "version.h"
#include "gm_busDefs.h"
#include "storage.h"

#ifndef PT_MAXENDPOINTS
#define PT_MAXENDPOINTS 16
#endif

class TParaTable : private TEpSysDefs
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
        const char* name;
    } endpoint_t;

    void init(uint32_t aUniqueId, devType_t aDevType, TStorage* aStorage = 0);
    void addEndpoint(endpoint_t* aEndpoint_t);
    endpoint_t* findEp(uint16_t baseInd);
    endpoint_t* getEpLL() {return &mSysEndpoint;};
    void setPara(uint16_t aRegAdr, uint32_t aData);
    bool getPara(uint16_t aRegAdr, uint32_t *aData);
    bool getParaAdr(uint16_t aRegAdr, uint32_t** aPraRec);

    void loadPara();
    void storePara();

    void loadDefault(uint16_t aRegAdr = CInvalidReg);

private:
    TStorage* mStorage;

    paraRec_t* findPara(uint16_t index);

    paraRec_t mSysPara[cParaListLength];
    endpoint_t mSysEndpoint;

    paraRec_t mEpListPara[PT_MAXENDPOINTS];
    endpoint_t mEpListEndpoint;
    paraDef_t mEpListEndpointDefs[PT_MAXENDPOINTS];

    endpoint_t* mStoreLoadEp;
    uint32_t mStoreLoadInd;
    uint32_t mNVCheckSum;
    uint32_t mNVLen;

    void calcNVCheckSum(uint32_t* aCheckSum, uint32_t* aNVParaCnt);
    static bool storeParaCb(void* aArg, uint32_t* aData, uint32_t aLen);
    static bool loadParaCb(void* aArg, uint32_t* aData, uint32_t aLen);
    static void paraSaveCb(void* aCbArg, struct paraRec_s* aPParaRec, bool aWrite);
    static void paraStartCb(void* aCbArg, struct paraRec_s* aPParaRec, bool aWrite);
    void loadDefault(paraRec_t* aPara);
};

#endif /* PARATABLE_H_ */
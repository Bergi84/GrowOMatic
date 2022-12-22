#ifndef PARATABLE_H_
#define PARATABLE_H_

#include "stdint.h"
#include "version.h"
#include "gm_bus.h"
#include "storage.h"

#ifndef PT_MAXENDPOINTS
#define PT_MAXENDPOINTS 16
#endif

class TParaTable
{
public:
    TParaTable();

    static constexpr uint32_t PARA_FLAG_W =  0x00000001;     // is writable
    static constexpr uint32_t PARA_FLAG_R =  0x00000002;     // is readable
    static constexpr uint32_t PARA_FLAG_RW = 0x00000003;    // is write and readable
    static constexpr uint32_t PARA_FLAG_NV = 0x00000004;    // is non volatile stored
    static constexpr uint32_t PARA_FLAG_S =  0x00000008;     // is scopable
    static constexpr uint32_t PARA_FLAG_FR = 0x00000010;   // call update callback before read 
    static constexpr uint32_t PARA_FLAG_FW = 0x00000020;   // call update callback after write
    static constexpr uint32_t PARA_FLAG_P =  0x00000040;    // parameter is a pointer

    typedef struct paraRec_s
    {
        union{
            uint32_t para;
            uint32_t* pPara;
        };
        void (*pFAccessCb)(void* aCbArg, struct paraRec_s* aPParaRec, bool aWrite);
        void *cbArg;
        uint32_t flags;
    } paraRec_t;

    typedef struct endpoint_s
    {
        union {
            struct {
                uint16_t startIndex;
                uint16_t type;
            };
            uint32_t epId;
        };
        uint32_t length;
        paraRec_t* para;
        struct endpoint_s* next;
    } endpoint_t;

    void init(uint32_t aUniqueId, devType_t aDevType, TStorage* aStorage);
    void addEndpoint(endpoint_t* aEndpoint_t);
    void setPara(uint16_t aRegAdr, uint32_t aData);
    bool getPara(uint16_t aRegAdr, uint32_t *aData);
    bool getParaAdr(uint16_t aRegAdr, uint32_t** aPraRec);

private:
    paraRec_t* findPara(uint16_t index);

    static constexpr uint32_t mSysParaLen = 6; 

    paraRec_t mSysPara[mSysParaLen];
    endpoint_t mSysEndpoint;

    paraRec_t mEpListPara[PT_MAXENDPOINTS];
    endpoint_t mEpListEndpoint;
};

#endif /* PARATABLE_H_ */
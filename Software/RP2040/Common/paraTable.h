#ifndef PARATABLE_H_
#define PARATABLE_H_

#include "pico/stdlib.h"

class TParaTable
{


public:
    static constexpr uint32_t PARA_FLAG_W = 0x00000001;
    static constexpr uint32_t PARA_FLAG_R = 0x00000002;
    static constexpr uint32_t PARA_FLAG_RW = 0x00000003;
    static constexpr uint32_t PARA_FLAG_NV = 0x00000004;
    static constexpr uint32_t PARA_FLAG_S = 0x00000008;

    typedef struct paraRec_s
    {
        uint32_t* pPara;
        void (*pFAccessCb)(void* aCbArg, struct paraRec_s* aPParaRec, bool aWrite);
        void *cbArg;
        uint32_t flags;
    } paraRec_t;

    typedef struct subTable_s
    {
        uint16_t startIndex;
        uint16_t length;
        paraRec_t* para;
        struct subTable_s* next;
    } subTable_t;

    void init();
    void addSubTable(subTable_t* aSubTable);
    void setPara(uint16_t aRegAdr, uint32_t aData);
    bool getPara(uint16_t aRegAdr, uint32_t *aData);
    bool getParaAdr(uint16_t aRegAdr, uint32_t** aPraRec);

private:
    subTable_t* mTableRoot;
    paraRec_t* findPara(uint16_t index);

};

#endif /* PARATABLE_H_ */
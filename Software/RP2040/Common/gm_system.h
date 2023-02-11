#ifndef GM_SYSTEM_H_
#define GM_SYSTEM_H_

#include "stdint.h"
#include "version.h"
#include "gm_busDefs.h"
#include "paraTable.h"

class TSystem : private TEpSysDefs
{
public:
    TSystem();

    void init(uint32_t aUniqueId, devType_t aDevType, TParaTable* aParaTable);

private:
    TParaTable::paraRec_t mSysPara[cParaListLength];
    TParaTable::endpoint_t mSysEndpoint;
    TParaTable* mPT;

    char mDevName[16];

    static void paraSaveCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite);
    static void paraStartCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite);
};

#endif 
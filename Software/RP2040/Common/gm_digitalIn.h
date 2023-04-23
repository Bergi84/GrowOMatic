#ifndef GM_DIGITALIN
#define GM_DIGITALIN

#include "stdint.h"
#include "gm_busDefs.h"
#include "paraTable.h"

class GM_digitalIn : public TEpDigitalInDefs
{
private:
    TParaTable::paraRec_t mPara[cParaListLength];
    TParaTable::endpoint_t mEp;
    TParaTable* mPT;

    uint32_t* mReg;
    uint32_t mChMsk;

    static void paraIn(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite);

public:
    GM_digitalIn();

    void init(TParaTable* aPT, uint32_t aBaseRegAdr, uint32_t* aInReg, uint32_t aInCh);
};

#endif
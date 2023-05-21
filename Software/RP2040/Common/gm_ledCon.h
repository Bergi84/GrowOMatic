#ifndef GM_LEDCON_H_
#define GM_LEDCON_H_

#include "stdint.h"
#include "gm_busDefs.h"
#include "paraTable.h"

class GM_ledCon : public TEpLedConDefs
{
private:
    TParaTable *mPT;

    uint32_t mPwmSlice;
    uint32_t mPwmCh;

    uint16_t mPeriod;
    int16_t mPwmComp;
    uint32_t mScale;

    TParaTable::paraRec_t mPara[cParaListLength];
    TParaTable::endpoint_t mEp;

    static void paraDimCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite);
    static void paraFreqCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite);

public:
    GM_ledCon();

    void init(TParaTable* aPT, uint32_t aBaseRegAdr,uint32_t mGpio);

    char* getEpName() {return mEp.epName;};
};

#endif
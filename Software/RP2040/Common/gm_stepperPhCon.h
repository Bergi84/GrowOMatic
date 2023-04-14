#ifndef GM_STEPPERCON_H_
#define GM_STEPPERCON_H_

#include "stdint.h"
#include "paraTable.h"
#include "rp_timerServer.h"
#include "gm_busDefs.h"

class TStepperPhCon : public TEpStepperConDefs {
private:
    static constexpr uint32_t cStepTable[] = {
        0x00000001,
        0x00000002,
        0x00000004,
        0x00000008
    };

    TParaTable::paraRec_t mPara[cParaListLength];
    TParaTable::endpoint_t mEp;
    TParaTable* mPT;
    TTimerServer* mTS;

    uint32_t mOutMsk;
    uint32_t mOutShift;
    int32_t mAktSpeed;
    int32_t mInitSpeed;

    void (*mSetOut)(void* aArg, uint32_t aOut, uint32_t aMsk);
    void* mSetOutArg;
    TTimer* mTimer;

    static uint32_t timerCb(void *aArg);

    static void paraSpeedCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite);
    static void paraAccCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite);

public:
    TStepperPhCon();
    void init(TParaTable *aPT, TTimerServer *aTS, uint16_t aBaseRegAdr);
    void setOutCb(uint32_t aOutShift, void (*aOutFunc)(void*, uint32_t, uint32_t), void* aArg);
    char* getEpName() {return mEp.epName;};
};

#endif
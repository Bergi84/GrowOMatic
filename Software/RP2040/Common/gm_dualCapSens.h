#ifndef GM_DUALCAPSENSE_H_
#define GM_DUALCAPSENSE_H_

#include "stdint.h"
#include "gm_busDefs.h"
#include "rp_capSens.h"
#include "paraTable.h"
#include "sequencer_armm0.h"

#define gpio_dls_capSensExc_base    0
#define gpio_dls_capSens_base       2
#define gpio_dls_capSens_chNo       22

#define cap_ch_as_p         0
#define cap_ch_as_n         21
#define cap_ch_sens_p       1
#define cap_ch_sens_n       11

class GM_dualCapSense : public TEpCapLevelDefs
{
public:
    GM_dualCapSense();

    void init (TParaTable* aParaTable, uint32_t aBaseAdr, TSequencer* aSeq_c1 = 0);

private:
    TCapSens mCapSens;

    TParaTable::paraRec_t mPara[2][cParaListLength];
    TParaTable::endpoint_t mEndpoint[2];
    TParaTable* mPT;

    static void paraLevelCb(void* aCbArg, struct TParaTable::paraRec_s* aPParaRec, bool aWrite);
    static void paraValCb(void* aCbArg, struct TParaTable::paraRec_s* aPParaRec, bool aWrite);
    static void paraCalCb(void* aCbArg, struct TParaTable::paraRec_s* aPParaRec, bool aWrite);
};

#endif

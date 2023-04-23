#ifndef GM_LEAKSENSOR_H_
#define GM_LEAKSENSOR_H_

#include "stdint.h"
#include "gm_busDefs.h"
#include "paraTable.h"
#include "rp_adc.h"

class GM_leakSensor : public TEpLeakSensorDefs
{
private:
    TParaTable::paraRec_t mPara[cParaListLength];
    TParaTable::endpoint_t mEp;
    TParaTable* mPT;

    TAdc* mAdc; 
    uint32_t mCh;

    static constexpr uint32_t cAdcRes = 4095;
    static constexpr uint32_t cGain = 11;
    static constexpr uint32_t cRpull = 1000000;

    static void paraResSec(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite);

public:
    GM_leakSensor();

    void init(TParaTable* aPT, uint32_t aBaseRegAdr, TAdc *aAdc, uint32_t aCh);
};

#endif
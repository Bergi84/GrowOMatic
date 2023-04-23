#ifndef GM_PULSSESNOR_H_
#define GM_PULSSESNOR_H_

#include "stdint.h"
#include "gm_busDefs.h"
#include "paraTable.h"
#include "rp_timerServer.h"
#include "hardware\pio.h"
#include "rp_pwmCapture.h"

class GM_pulsSensor : public TEpPulsSensorDefs
{
private:
    TParaTable::paraRec_t mPara[cParaListLength];
    TParaTable::endpoint_t mEp;
    TParaTable* mPT;

    TPwmCapture mPwmC;

    static void paraPulsPerMin(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite);

public:
    GM_pulsSensor();

    void init(TParaTable* aPT, uint32_t aBaseRegAdr, TTimerServer* aTS, TDmaIrqMng* aIrqMng, uint32_t aGpio, PIO aPio, uint32_t aSmNo);

    inline char* getEpName() {return mEp.epName;};
};


#endif
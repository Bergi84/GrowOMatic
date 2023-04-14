#ifndef GM_DCMOTORCON_H_
#define GM_DCMOTORCON_H_

#include "stdint.h"
#include "paraTable.h"
#include "rp_timerServer.h"
#include "gm_busDefs.h"
#include "rp_adc.h"

class gm_dcMotorCon : public TEpDcMotorConDefs
{
public:
    gm_dcMotorCon();

    void init(TParaTable* aPT, uint32_t aBaseRegAdr, TTimerServer* aTS);
    void setGpioPwmEn(uint32_t aGpioPwm, uint32_t aGpioEn);
    void setGpioPwm(uint32_t aGpioPwm1, uint32_t aGpioPwm2);
    void setCurAdc(TAdc* aAdc, uint32_t aCh, uint32_t aFullScale);
    void setGpioCurLim(uint32_t aGpio, uint32_t aFullScale);

    char* getEpName() {return mEp.epName;};

private:
    bool mUni;
    uint32_t mPwmSlice;
    uint32_t mPwmCh1;
    uint32_t mPwmCh2;
    uint32_t mGpioEn;
    uint32_t mGpioPwm;

    uint16_t mPeriod;
    int16_t mPwmComp;
    int32_t mPwmAkt;
    uint32_t mPwmRampInc;

    TParaTable::paraRec_t mPara[cParaListLength];
    TParaTable::endpoint_t mEp;
    TParaTable* mPT;
    TTimerServer* mTS;
    TAdc* mAdc;

    TTimer* aRampTimer;

    uint32_t mCurAdcCh;
    uint32_t mCurAdcScale;

    static constexpr uint32_t cCurLimPeriod = 6250;
    uint32_t mCurLimSlice;
    uint32_t mCurLimCh;
    uint32_t mCurLimScale;

    static void paraPwmDutyCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite);
    static void paraCurCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite);
    static void paraCurLimCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite);
    static void paraFreq(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite);
    static void paraRampCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite);

    static uint32_t rampTimerCb(void* aArg);
};

#endif
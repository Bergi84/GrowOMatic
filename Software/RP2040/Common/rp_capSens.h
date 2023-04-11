#ifndef CAPSENS_H_
#define CAPSENS_H_

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "rp_capSens.pio.h"
#include "hardware/irq.h"
#include "irqVeneer.h"
#include "sequencer_armm0.h"

#define MAX_SENSE_PIN_CNT   32

class TCapSens {
private:
    PIO mPio;
    uint mExcPinBase;
    uint mSensPinBase;
    uint mSensPinCount;
    uint mSensPinPolMsk;

    uint mSumBuf[4];
    uint mSumCnt;
    uint mSampleCnt;
    uint mSumShift;
    uint mMaxShift;
    uint mAktivPin;

    uint mMaxChgCnt;

    irqVeneer_t mIrqVeneer;

    inline void setSensCh(uint sm, uint gpio);
    inline void setNextSensPins();
    static void setIrqHandler(void* aArg);
    static void __time_critical_func(irqHandler)(void *aArg);

public:
    uint mCapVal[MAX_SENSE_PIN_CNT];
    uint32_t mDebEmptyCall;

    void init(PIO aPio, uint aExcPinBase, uint aSensPinBase, uint aSensPinCnt, uint aSensPinPolMsk, uint aMaxChgCnt, uint aDchgCnt, TSequencer* aSeq_c1 = 0);

    void setOverSampling(uint aSampleCnt);
};

#endif /* CAPSENS_H_ */

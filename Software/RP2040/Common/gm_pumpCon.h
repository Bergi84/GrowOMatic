#ifndef PERISTALTICPUMPCON_H_
#define PERISTALTICPUMPCON_H_

#include "rp_ioExpander.h"
#include "paraTable.h"
#include "rp_timerServer.h"
#include "gm_stepperPhCon.h"
#include "gm_dcMotorCon.h"
#include "rp_adc.h"
#include "rp_dmaIrqMng.h"
#include "gm_leakSensor.h"
#include "gm_pulsSensor.h"
#include "gm_ledCon.h"

#define gpio_pc_spiEn               8
#define gpio_pc_spiCs               9
#define gpio_pc_spiClk              10
#define gpio_pc_spiD                11
#define gpio_pc_pwmP0               12
#define gpio_pc_enP0                13
#define gpio_pc_pwmP1               14
#define gpio_pc_enP1                15
#define gpio_pc_iRefP0              16
#define gpio_pc_iRefP1              17
#define gpio_pc_flowPuls1           21
#define gpio_pc_flowPuls0           23
#define gpio_pc_ledPwm              24
#define gpio_pc_iSens0              26
#define gpio_pc_iSens1              27
#define gpio_pc_leakSens            29  

#define PPC_MAX_STEPPER             8
#define PPC_DCMOTORS                2
#define PPC_PULSSENSOR              2

class GM_pumpCon {
private:
    TParaTable *mPT;
    TTimerServer *mTS;


    TIoExpander mIOE;
    TStepperPhCon mStepper[PPC_MAX_STEPPER];

    TDmaIrqMng mIrqMng;
    TAdc mAdc;
    GM_dcMotorCon mDcMotor[PPC_DCMOTORS];

    GM_pulsSensor mPulsSensor[PPC_PULSSENSOR];
    GM_leakSensor mLeakSensor;
    GM_ledCon mLedCon;

public:
    GM_pumpCon();
    void init(TParaTable *aPT, TTimerServer *aTS_c1, TSequencer* aSeq_c1);
};

#endif
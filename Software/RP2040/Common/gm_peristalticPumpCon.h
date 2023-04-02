#ifndef PERISTALTICPUMPCON_H_
#define PERISTALTICPUMPCON_H_

#include "ioExpander.h"
#include "paraTable.h"
#include "rp_timerServer.h"
#include "gm_stepperCon.h"

#define gpio_pc_spiEn               8
#define gpio_pc_spiCs               9
#define gpio_pc_spiClk              10
#define gpio_pc_spiD                11

#define PPC_MAX_STEPPER             8

class gm_perestalticPumpCon {
private:
    TParaTable *mPT;
    TTimerServer *mTS;
    TIoExpander mIOE;

    TStepperCon mStepper[PPC_MAX_STEPPER];

public:
    gm_perestalticPumpCon();
    void init(TParaTable *aPT, TTimerServer *aTS);
};

#endif
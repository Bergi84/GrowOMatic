#include "gm_stepperPhCon.h"
#include <string.h>
#include "pico/float.h"

TStepperPhCon::TStepperPhCon() :
mPara( (TParaTable::paraRec_t[cParaListLength]) {
    [PARA_SPEED] =          {.para = 0,     .pFAccessCb = paraSpeedCb,  .cbArg = this,  .defs = &cParaList[PARA_SPEED]},
    [PARA_POS] =            {.para = 0,     .pFAccessCb = 0,            .cbArg = 0,     .defs = &cParaList[PARA_POS]},
    [PARA_MAXSPEED] =       {.para = 1000,  .pFAccessCb = 0,            .cbArg = 0,     .defs = &cParaList[PARA_MAXSPEED]},
    [PARA_ACCEL]  =         {.para = 200,   .pFAccessCb = paraAccCb,    .cbArg = this,  .defs = &cParaList[PARA_ACCEL]},
    [PARA_STEPS_PER_REV] =  {.para = 4096,  .pFAccessCb = 0,            .cbArg = 0,     .defs = &cParaList[PARA_STEPS_PER_REV]},
}),
mEp( (TParaTable::endpoint_t) {  
    { { 
        .baseInd = CPeriPumpBaseRegAdr,
        .type = (uint16_t)EPT_STEPPERCON    
    } }, 
    .length = cParaListLength, 
    .para = mPara,
    .next = 0,
    .typeName = cTypeName   
})
{
    strncpy(mEp.epName, cTypeName, sizeof(mEp.epName));
}

void TStepperPhCon::init(TParaTable *aPT, TTimerServer *aTS, uint16_t aBaseRegAdr, uint32_t mIndex)
{
    mPT = aPT;
    mTS = aTS;

    mEp.epId.baseInd = aBaseRegAdr;

    uint32_t len = strlen(mEp.epName);

    mEp.epName[len] = mIndex + '0';
    mEp.epName[len + 1] = 0; 
    mPT->addEndpoint(&mEp);

    mInitSpeed = float2int(sqrtf(2*mPara[PARA_ACCEL].para));

    mTimer = mTS->getTimer(timerCb, this);
}

void TStepperPhCon::setOutCb(uint32_t aOutShift, void (*aOutFunc)(void*, uint32_t, uint32_t), void* aArg)
{
    mOutShift = aOutShift;
    mOutMsk = 0x0000000F << aOutShift; 
    mSetOut = aOutFunc;
    mSetOutArg = aArg;

    mSetOut(mSetOutArg, 0, mOutMsk);
}

void TStepperPhCon::paraSpeedCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite)
{
    TStepperPhCon* pObj = (TStepperPhCon*) aCbArg;

    int32_t vLoc = (int32_t)pObj->mPara[PARA_SPEED].para;

    // limit speed
    if(abs(vLoc) > pObj->mPara[PARA_MAXSPEED].para)
    {
        if(vLoc > 0)
            vLoc = pObj->mPara[PARA_MAXSPEED].para;
        else
            vLoc = -1 * pObj->mPara[PARA_MAXSPEED].para;

        pObj->mPara[PARA_SPEED].para = vLoc;
    }

    // start timer if not runing
    if(!pObj->mTimer->isAktive())
    {
        // enable outputs
        pObj->mSetOut(pObj->mSetOutArg, cStepTable[pObj->mPara[PARA_POS].para & 0x00000002] << pObj->mOutShift, pObj->mOutMsk);

        if( pObj->mInitSpeed > abs( vLoc ) )
            pObj->mInitSpeed = abs( vLoc );

        if(vLoc > 0)
            pObj->mAktSpeed = pObj->mInitSpeed;
        else
            pObj->mAktSpeed = -1*pObj->mInitSpeed;

        pObj->mTimer->setTimer((1000000*1000)/(pObj->mInitSpeed*pObj->mPara[PARA_STEPS_PER_REV].para));
    }
}

void TStepperPhCon::paraAccCb(void* aCbArg, TParaTable::paraRec_t* aPParaRec, bool aWrite)
{
    TStepperPhCon* pObj = (TStepperPhCon*) aCbArg;

    if(pObj->mPara[PARA_ACCEL].para == 0)
        pObj->mPara[PARA_ACCEL].para = 20;

    pObj->mInitSpeed = float2int(sqrtf(2*pObj->mPara[PARA_ACCEL].para));
}

uint32_t TStepperPhCon::timerCb(void *aArg)
{
    TStepperPhCon* pObj = (TStepperPhCon*) aArg;

    
    int32_t vLoc = (int32_t)pObj->mPara[PARA_SPEED].para;

    if(pObj->mAktSpeed == 0 && vLoc == 0)
    {
        // disable Output
        pObj->mSetOut(pObj->mSetOutArg, 0, pObj->mOutMsk);
        return 0;
    }
    else
    {
        // set output
        if(pObj->mAktSpeed > 0)
            pObj->mPara[PARA_POS].para++;
        else
            pObj->mPara[PARA_POS].para--;
        pObj->mSetOut(pObj->mSetOutArg, cStepTable[pObj->mPara[PARA_POS].para & 0x00000003] << pObj->mOutShift, pObj->mOutMsk);

        // calculate next call back time, estimation without sqrt
        uint32_t time;
        if(pObj->mAktSpeed >= pObj->mInitSpeed || pObj->mAktSpeed == vLoc)
            time = 1000000/pObj->mAktSpeed;
        else
            time = 1000000/pObj->mInitSpeed;

        int32_t newSpeed;

        if(pObj->mAktSpeed < vLoc)
        {
            newSpeed = pObj->mAktSpeed + time * pObj->mPara[PARA_ACCEL].para;
            if(newSpeed >= vLoc)
                newSpeed = vLoc;
        }
        else
        {
            newSpeed = pObj->mAktSpeed - time * pObj->mPara[PARA_ACCEL].para;
            if(newSpeed <= vLoc)
                newSpeed = vLoc;
        }
        pObj->mAktSpeed = newSpeed;

        if(pObj->mAktSpeed == 0 && vLoc == 0)
            return (1000*1000000)/(pObj->mInitSpeed*pObj->mPara[PARA_STEPS_PER_REV].para);
        else
            return (1000*time)/(pObj->mPara[PARA_STEPS_PER_REV].para);
    }
}
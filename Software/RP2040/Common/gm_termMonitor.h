#ifndef GM_TERMMONITOR_H_
#define GM_TERMMONITOR_H_

#include "termApp.h"
#include "gm_PathEle.h"
#include "sequencer_armm0.h"
#include "rp_timerServer.h"

class gm_termMonitor : public TTermApp
{
public:
    gm_termMonitor();
    virtual ~gm_termMonitor();

    void init(TSequencer* aSeq, TTimerServer* aTS);

private:
    static constexpr char mName[] = "monitor"; 

    TSequencer* mSeq;
    TTimerServer* mTS;

    TTimer* mTimer; 

    TPathEle mPathEle;
    TPathEle mSubPathEle;
    uint32_t mSubPathInd;
    bool mSubReqAktiv;

    uint8_t mWorkerTaskId;

    virtual void start(uint8_t* aStartArg);
    virtual void parse(uint8_t aChar);
    virtual void exit();

    void reqNext();

    static uint32_t timerCb(void* aArg);

    static void workerTask(void* aArg);

    static void reqCb(void* aArg, uint32_t* aVal, errCode_T aStatus);
};

#endif
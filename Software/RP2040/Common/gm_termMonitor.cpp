#include "gm_termMonitor.h"
#include "gm_termPathMng.h"
#include "hardware/timer.h"
#include "terminal.h"
#include <stdio.h>
#include <stdlib.h>

gm_termMonitor::gm_termMonitor()
{
    mKeyPhrase = mName;
}

gm_termMonitor::~gm_termMonitor()
{

}

void gm_termMonitor::init(TSequencer* aSeq, TTimerServer* aTS)
{
    mSeq = aSeq;
    mTS = aTS;
    mTimer = mTS->getTimer(timerCb, this);
    mSubReqAktiv = false;
    mSeq->addTask(mWorkerTaskId, workerTask, this);
}

void gm_termMonitor::start(uint8_t* aStartArg)
{
    uint32_t pos = findNextS((char*) aStartArg);

    char* pathStr;
    if(pos != 0)
        pathStr = (char*)aStartArg;
    else
        pathStr = 0;

    GM_termPathMng* pathMng = (GM_termPathMng*) getPathMng();
    pathMng->getPathObj(pathStr, pos, &mPathEle);

    if(!mPathEle.isEndPoint())
    {
        done();
    }

    mSeq->queueTask(mWorkerTaskId);
    mTimer->setTimer(100000);
}

void gm_termMonitor::parse(uint8_t aChar)
{

}

void gm_termMonitor::exit()
{
    mSubReqAktiv = false;
    mTimer->stopTimer();
}

uint32_t gm_termMonitor::timerCb(void* aArg)
{
    gm_termMonitor* pObj = (gm_termMonitor*) aArg;
    if(!pObj->mSubReqAktiv)
        pObj->mSeq->queueTask(pObj->mWorkerTaskId);

    return 100000;
}

void gm_termMonitor::workerTask(void* aArg)
{
    gm_termMonitor* pObj = (gm_termMonitor*) aArg;

    // clear screen
    pObj->putChar(CTRLSYM_POS00);
    pObj->putChar(CTRLSYM_ES);

    pObj->mSubReqAktiv = true;
    pObj->mSubPathInd = 0;
    pObj->reqNext();
}

void gm_termMonitor::reqCb(void* aArg, uint32_t* aVal, errCode_T aStatus)
{
    gm_termMonitor* pObj = (gm_termMonitor*) aArg;

    if(!pObj->mSubReqAktiv)
        return;

    if(aStatus != EC_SUCCESS)
    {
        pObj->printErr(aStatus);
    }
    else
    {
        const char* varName = pObj->mSubPathEle.getName();

        // print readed value
        pObj->putString(varName, strlen(varName));
        pObj->putString(" = ", 3);

        char printBuf[16];
        snprintf(printBuf, 16, "%u", *aVal);
        pObj->putString(printBuf, 16);
        pObj->putChar('\r');
        pObj->putChar('\n');
    }

    pObj->reqNext();
}

void gm_termMonitor::reqNext()
{
    if(!mSubReqAktiv)
        return;

    while(1)
    {
        errCode_T ec = mPathEle.getSubObj(mSubPathInd, &mSubPathEle);
        mSubPathInd++;

        if(ec == EC_SUCCESS)
        {
            if((mSubPathEle.getPer() & PARA_FLAG_S) != 0)
            {
                mSubPathEle.getValue(reqCb, this);
                return;
            }
        }
        else
        {
            mSubReqAktiv = false;
            return;
        }
    }
}
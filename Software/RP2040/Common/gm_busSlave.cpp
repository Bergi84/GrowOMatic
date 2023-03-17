#include "gm_busSlave.h"

#include "rp_debug.h"

GM_busSlave::GM_busSlave() : GM_BusDefs()
{
    mState = S_IDLE;
    mInit = false;
}

void GM_busSlave::init(TUart *aUart0, TUart *aUart1, TParaTable *aParaTable, TSequencer* aSeq, TTimerServer* aTimerServer)
{
    if(!mInit)
    {
        gDebug.resetPin(2);

        mByteTimeUs = (9999999 + mBaudRate)/mBaudRate;
        mByteTimeoutUs = (mByteTimeUs*19) >> 4;     // 118,75% of byte time
        mCom[0].errCnt = 0;
        mCom[1].errCnt = 0;
        mCom[0].byteCnt = 0;
        mCom[1].byteCnt = 0;
        mCom[0].sec = false;
        mCom[1].sec = false;
        mCom[0].reqR = false;
        mCom[1].reqR = false;
        mCom[0].otherCom = &mCom[1];
        mCom[1].otherCom = &mCom[0];
        mCom[0].uart = aUart0;
        mCom[1].uart = aUart1;

        mCom[0].uart->config(mBaudRate, UP_NONE);
        mCom[1].uart->config(mBaudRate, UP_NONE);

        uint8_t tmp;
        while(mCom[0].uart->rxPending())
            mCom[0].uart->rxChar(&tmp);
        while(mCom[1].uart->rxPending())
            mCom[1].uart->rxChar(&tmp);

        mCom[0].uart->disableFifo(true);
        mCom[1].uart->disableFifo(true);
        mCom[0].uart->disableTx(true);
        mCom[1].uart->disableTx(true);
        mCom[0].uart->installRxCb(rx0CbWrapper, (void*)this);
        mCom[1].uart->installRxCb(rx1CbWrapper, (void*)this);

        mParaTable = aParaTable;

        uint32_t uid;
        aParaTable->getPara(CSystemBaseRegAdr + TEpSysDefs::PARA_UID, &uid);
        mCrcInitValW = crcCalc(uid, mSdW);
        mCrcInitValR = crcCalc(uid, mSdR);
        mCrcInitValB = crcCalc(0, mSdW);

        mSeq = aSeq;
        aSeq->addTask(mParaRWTaskId, paraRW, (void*)this);

        mTS = aTimerServer;
        mTimeoutTimer = mTS->getTimer(timeOutCb, this);
        mRegTimeoutTimer = mTS->getTimer(regTimeOutCb, this);

        mInit = true;

        mRegTimeOutFlag = false;
    }
};

void GM_busSlave::deinit()
{
    if(mInit)
    {
        // todo: wait until S_IDLE before disable bus
        mCom[0].uart->installRxCb(0, 0);
        mCom[1].uart->installRxCb(0, 0);    

        mSeq->delTask(mParaRWTaskId);
        mTimeoutTimer->delTimer();
        mRegTimeoutTimer->delTimer();

        mInit = false;
    }
}   

void GM_busSlave::rxCb (com_t* aCom)
{
    uint8_t byte;

    if(!aCom->uart->rxPending())
        while(1);

    uint32_t status = save_and_disable_interrupts();

    aCom->uart->rxChar(&byte);
    aCom->byteCnt++;

    if(!aCom->sec)
    {
        gDebug.setPin(1); 
            
        switch (mState)
        {
        case S_IDLE:
            
            // both ports are in read mode, if we find a start deliminater
            // we can set the direction an start relaying
            if(byte == mSdW || byte == mSdR)
            {           
                mState = S_DEVADR;
                mWrite = (byte == mSdW);
                aCom->otherCom->sec = true;

                aCom->otherCom->uart->disableTx(false);
                aCom->otherCom->uart->txChar(byte);
                
                mTimeoutTimer->setTimer(mByteTimeoutUs);
            }
            else
            {
                aCom->errCnt++;
            }
            break;

        case S_DEVADR:
            mTimeoutTimer->stopTimer();
            
            if(byte == 0 || byte == CBroadcastAdr)
            {
                // request belongs to this device so we must decode the hole massage
                // we send a the invalid Addr to the next device to signal abort
                mState = S_REGADR;
                mBroadCast = (byte == CBroadcastAdr);
                if(mBroadCast)
                {
                    aCom->otherCom->uart->txChar(CBroadcastAdr);
                    mCrcCalc.dw = mCrcInitValB;
                }
                else
                {
                    aCom->otherCom->uart->txChar(CInvalidAdr);
                    if(mWrite)
                        mCrcCalc.dw = mCrcInitValW;
                    else
                        mCrcCalc.dw = mCrcInitValR;
                    
                }
                mCrcCalc.dw = crcCalc(mCrcCalc.dw, byte);

                mTimeoutTimer->setTimer(mByteTimeoutUs);
            }
            else
            {
                // the message belongs to a other device
                if(byte == CInvalidAdr)
                {
                    // we got the invalid Adr which means the message belongs to a device before this
                    // and we can abort the relaying
                    aCom->otherCom->uart->txChar(CInvalidAdr);
                    mState = S_IDLE;
                    aCom->byteCnt = 0;
                }
                else
                {
                    // we got an adr which belongs to a device beyond this
                    mState = S_FWD;
                    aCom->otherCom->uart->txChar(byte - 1);

                    mTimeoutTimer->setTimer(mByteTimeoutUs);

                    // calculate turnaround timout = transfaretime + 50% * bytetime
                    mTurnAroundTimeout = mByteTimeoutUs * 2 * (byte - 1) + ((mByteTimeUs * 3) >> 1);
                }
            }
            break;

        case S_REGADR:
            mTimeoutTimer->stopTimer();
            mCrcCalc.dw = crcCalc(mCrcCalc.dw, byte);

            if(mBroadCast)
                aCom->otherCom->uart->txChar(byte);

            if(aCom->byteCnt == 4)
            {
                mState = S_DATA;
                mRegAdr.b[1] = byte; 

                if(mWrite)
                {
                    mTimeoutTimer->setTimer(mByteTimeoutUs);
                }
                else
                {
                    // in read case we must turnaround the direction
                    // this is done in idle task
                    mRegTimeOutFlag = false;
                    aCom->reqR = true;
                    aCom->uart->disableTx(false);
                    mSeq->queueTask(mParaRWTaskId);
                    mRegTimeoutTimer->setTimer(mByteTimeUs>>2);
                }                    
            }
            else
            {
                mRegAdr.b[0] = byte;  

                mTimeoutTimer->setTimer(mByteTimeoutUs);
            }
            break;

        case S_DATA:
            mTimeoutTimer->stopTimer();

            if(aCom->byteCnt == 8)
            {
                mState = S_CRC;
            }

            if(mWrite)
            {
                if(mBroadCast)
                    aCom->otherCom->uart->txChar(byte);

                mData.b[aCom->byteCnt - 5] = byte;
                mCrcCalc.dw = crcCalc(mCrcCalc.dw, byte);
                mTimeoutTimer->setTimer(mByteTimeoutUs);
            } 
            else
            {
                if(aCom->byteCnt < 8)
                {
                    // send byte 1, 2 ,3 
                    aCom->uart->txChar(mData.b[aCom->byteCnt - 4]);
                    mTimeoutTimer->setTimer(mByteTimeoutUs);
                    mCrcCalc.dw = crcCalc(mCrcCalc.dw, mData.b[aCom->byteCnt - 4]);
                }
                else
                {
                    // send byte 0 
                    // if the register address is invalid send the inverted checksum
                    if(mInvalidRegAdr)
                        aCom->uart->txChar(~mCrcCalc.b[aCom->byteCnt - 8]);
                    else
                        aCom->uart->txChar(mCrcCalc.b[aCom->byteCnt - 8]);

                    mTimeoutTimer->setTimer(mByteTimeoutUs);
                }
            }
            break;

        case S_CRC:           
            mTimeoutTimer->stopTimer();

            if(!mBroadCast && aCom->byteCnt == 12)
            {
                aCom->otherCom->uart->disableTx(true);
                aCom->otherCom->sec = false;
                aCom->otherCom->byteCnt = 0;
            }

            if(mWrite)
            {
                
                if(mBroadCast)
                    aCom->otherCom->uart->txChar(byte);

                mCrc.b[aCom->byteCnt - 9] = byte;
                if(aCom->byteCnt == 12)
                {
                    mBroadCast = false;
                    mState = S_IDLE;
                    aCom->byteCnt = 0;
                    if(mCrc.dw == mCrcCalc.dw)
                    {
                        gDebug.setPin(7);
                        mRegTimeOutFlag = false;
                        mSeq->queueTask(mParaRWTaskId);
                        

                        // if the request is not written fast enougth the following
                        // datagramm can overwrite data
                        mRegTimeoutTimer->setTimer(mByteTimeUs>>2);
                        gDebug.resetPin(7);
                    }
                }
                else
                {
                    mTimeoutTimer->setTimer(mByteTimeoutUs);
                }
            } 
            else
            {
                if(aCom->byteCnt == 12)
                {
                    mState = S_IDLE;
                    aCom->uart->disableTx(true);
                    aCom->byteCnt = 0;
                }
                else
                {
                    if(aCom->byteCnt < 12)
                    {
                        // send byte 1, 2, 3
                        // if the register address is invalid send the inverted checksum
                        if(mInvalidRegAdr)
                            aCom->uart->txChar(~mCrcCalc.b[aCom->byteCnt - 8]);
                        else
                            aCom->uart->txChar(mCrcCalc.b[aCom->byteCnt - 8]);

                        mTimeoutTimer->setTimer(mByteTimeoutUs);
                    } 
                }
            }
            break;

        case S_FWD:
            if(mWrite)
            {
                mTimeoutTimer->stopTimer();
                aCom->otherCom->uart->txChar(byte);
                if(aCom->byteCnt == 12)
                {
                    mState = S_IDLE;
                    aCom->byteCnt = 0;
                }
                else
                {
                    mTimeoutTimer->setTimer(mByteTimeoutUs);
                }
            }
            else
            {
                if(aCom->byteCnt == 12)
                {
                    mTimeoutTimer->stopTimer();
                    mState = S_IDLE;
                    aCom->uart->disableTx(true);
                    aCom->byteCnt = 0;
                    aCom->otherCom->byteCnt = 0;
                    aCom->otherCom->sec = false;
                }
                else
                {
                    // forword data until turnaround
                    if(aCom->byteCnt < 4)
                    {
                        mTimeoutTimer->stopTimer();
                        aCom->otherCom->uart->txChar(byte);
                        mTimeoutTimer->setTimer(mByteTimeoutUs);
                    }

                    if(aCom->byteCnt == 4)
                    {
                        mTimeoutTimer->stopTimer();
                        aCom->otherCom->uart->txChar(byte);
                        aCom->uart->disableTx(false);
                        mTimeoutTimer->setTimer(mByteTimeoutUs);
                    }
                }
            }
            break;
        
        default:
            aCom->errCnt++;
            break;
        }

        gDebug.resetPin(1);
    }
    else
    {
        gDebug.setPin(2);
        // we musst disable the uart after propagation of an recieved invalid address
        if(aCom->byteCnt == 2 && byte == CInvalidAdr && mState == S_IDLE)
        {
            aCom->uart->disableTx(true);
            aCom->sec = false;
            aCom->byteCnt = 0;
        }

        if(aCom->byteCnt == 4 && !mWrite && mState == S_FWD)
        {
            // turn around for read request needed
            mTimeoutTimer->stopTimer();
            aCom->uart->disableTx(true);
            mTimeoutTimer->setTimer(mTurnAroundTimeout);
        }

        if(aCom->byteCnt > 4 && !mWrite && mState == S_FWD)
        {
            mTimeoutTimer->stopTimer();
            // forwarding data from readrequest to primary uart
            aCom->otherCom->uart->txChar(byte);

            mTimeoutTimer->setTimer(mByteTimeoutUs);
        }

        if(aCom->byteCnt == 12)
        {
            // before we disable the direction we to check if there is already the next
            // transfare is running

            aCom->byteCnt = 0;
            if(mState == S_IDLE)
            {
                aCom->uart->disableTx(true);
                aCom->sec = false;
            };
        } 
        gDebug.resetPin(2); 
    }  
    restore_interrupts(status);
}


void GM_busSlave::rx0CbWrapper(void* aPObj)
{
    GM_busSlave* pObj = (GM_busSlave*) aPObj;

    pObj->rxCb(&pObj->mCom[0]);
}

void GM_busSlave::rx1CbWrapper(void* aPObj)
{
    GM_busSlave* pObj = (GM_busSlave*) aPObj;

    pObj->rxCb(&pObj->mCom[1]);
}

uint32_t GM_busSlave::timeOutCb(void* aPObj)
{
    GM_busSlave* pObj = (GM_busSlave*) aPObj;

    gDebug.setPin(3);

    if(pObj->mState != S_DATA || pObj->mRegAdr.w != 0)
    {
        if(pObj->mCom[0].reqR || pObj->mCom[1].sec)
            pObj->mCom[0].errCnt++;

        
        if(pObj->mCom[1].reqR || pObj->mCom[0].sec)
            pObj->mCom[1].errCnt++;
    }

    pObj->resetSlave();

    gDebug.resetPin(3);

    return 0;
}

void GM_busSlave::resetSlave()
{

    mCom[0].reqR = false;
    mCom[1].reqR = false;
    mCom[0].sec = false;
    mCom[1].sec = false;
    mCom[0].byteCnt = 0;
    mCom[1].byteCnt = 0;
    mCom[0].uart->disableTx(true);
    mCom[1].uart->disableTx(true);

    mState = S_IDLE;
}

uint32_t GM_busSlave::regTimeOutCb(void* aPObj)
{
    gDebug.setPin(4);

    GM_busSlave* pObj = (GM_busSlave*) aPObj;

    pObj->mRegTimeOutFlag = true;

    gDebug.resetPin(4);

    return 0;
}

void GM_busSlave::paraRW(void* aArg)
{
    gDebug.setPin(6);

    GM_busSlave* pObj = (GM_busSlave*) aArg;

    if(pObj->mWrite)
    {
        uint32_t status = save_and_disable_interrupts();
        if(!pObj->mRegTimeOutFlag)
        {
            pObj->mRegTimeoutTimer->stopTimer();
        }
        restore_interrupts(status);

        if(!pObj->mRegTimeOutFlag)
        {
            pObj->mParaTable->setPara(pObj->mRegAdr.w, pObj->mData.dw);
        }
        else
        {
            // the system needs to long for the answare
        }
    }
    else
    {
        // the first byte is directly loaded from send register to transmit engine
        pObj->mInvalidRegAdr = pObj->mParaTable->getPara(pObj->mRegAdr.w, &pObj->mData.dw) != EC_SUCCESS;


        uint32_t status = save_and_disable_interrupts();
        if(!pObj->mRegTimeOutFlag)
        {
            pObj->mRegTimeoutTimer->stopTimer();
        }
        restore_interrupts(status);

        if(!pObj->mRegTimeOutFlag)
        {   
            // start answer to read request
            // rest is done in irq

            pObj->mCrcCalc.dw = pObj->crcCalc(pObj->mCrcCalc.dw, pObj->mData.b[0]);
            if(pObj->mCom[0].reqR)
            {
                pObj->mCom[0].reqR = false;
                pObj->mCom[0].uart->txChar(pObj->mData.b[0]);
            }
            
            if(pObj->mCom[1].reqR)
            {
                pObj->mCom[1].reqR = false;
                pObj->mCom[1].uart->txChar(pObj->mData.b[0]);
            }

            pObj->mTimeoutTimer->setTimer(pObj->mByteTimeoutUs);
        }
        else
        {
            // the system needs to long for the answare
            pObj->resetSlave();
        }
    }

    gDebug.resetPin(6);
}
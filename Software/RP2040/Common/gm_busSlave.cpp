#include "gm_busSlave.h"

GM_busSlave::GM_busSlave() : GM_Bus()
{
    mState = S_IDLE;
}

void GM_busSlave::init(TUart *aUart0, TUart *aUart1, TParaTable *aParaTable, TSequencer* aSeq)
{
    crcInitTab();

    mByteTimeUs = (9999999 + mBaudRate)/mBaudRate;
    mByteTimeoutUs = (mByteTimeUs*18) >> 4;     // 112,5% of byte time
    mCom[0].errCnt = 0;
    mCom[1].errCnt = 0;
    mCom[0].byteCnt = 0;
    mCom[1].byteCnt = 0;
    mCom[0].sec = false;
    mCom[1].sec = false;
    mCom[0].otherCom = &mCom[1];
    mCom[1].otherCom = &mCom[0];
    mCom[0].uart = aUart0;
    mCom[1].uart = aUart1;

    mCom[0].uart->config(mBaudRate, UP_NONE);
    mCom[1].uart->config(mBaudRate, UP_NONE);
    mCom[0].uart->disableFifo(true);
    mCom[1].uart->disableFifo(true);
    mCom[0].uart->disableTx(true);
    mCom[1].uart->disableTx(true);
    mCom[0].uart->installRxCb(rx0CbWrapper, (void*)this);
    mCom[1].uart->installRxCb(rx1CbWrapper, (void*)this);

    mParaTable = aParaTable;
    aParaTable->getPara(0, &mCrcInitVal);

    mSeq = aSeq;
    aSeq->addTask(mParaRWTaskId, paraRW, (void*)this);
};

void GM_busSlave::deinit()
{
    mCom[0].uart->installRxCb(0, 0);
    mCom[1].uart->installRxCb(0, 0);    
}   

void GM_busSlave::rxCb(com_t* aCom)
{
    uint8_t byte;
    aCom->uart->rxChar(&byte);
    aCom->byteCnt++;

    if(!aCom->sec)
    {
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
                mCrc.dw = crcCalc(mCrcInitVal, byte);
                
                mTimeoutId = add_alarm_in_us(mByteTimeoutUs, timeOutCb, (void*) this, true);
            }
            else
            {
                aCom->errCnt++;
            }
            

        case S_DEVADR:
            cancel_alarm(mTimeoutId);
            mCrcCalc.dw = crcCalc(mCrcCalc.dw, byte);
            if(byte == 0)
            {
                // request belongs to this device so we must decode the hole massage
                // we send a the invalid Addr to the next device to signal abort
                mState = S_REGADR;
                aCom->otherCom->uart->txChar(CInvalidAdr);

                mTimeoutId = add_alarm_in_us(mByteTimeoutUs, timeOutCb, (void*) this, true);
            }
            else
            {
                // the message belongs to a other device
                if(byte == CInvalidAdr)
                {
                    // we got the invalid Adr which means the message belongs to a device before this
                    // and we can abort the relaying
                    mState = S_IDLE;
                    aCom->otherCom->uart->txChar(CInvalidAdr);
                    aCom->byteCnt = 0;
                }
                else
                {
                    // we got an adr which belongs to a device beyond this
                    mState = S_FWD;
                    aCom->otherCom->uart->txChar(byte - 1);

                    mTimeoutId = add_alarm_in_us(mByteTimeoutUs, timeOutCb, (void*) this, true);

                    // calculate turnaround timout = transfaretime + 50% * bytetime
                    mTurnAroundTimeout = mByteTimeoutUs * 2 * (byte - 1) + (mByteTimeUs * 3 >> 2);
                }
            }
            break;

        case S_REGADR:
            cancel_alarm(mTimeoutId);
            mCrcCalc.dw = crcCalc(mCrcCalc.dw, byte);
            if(aCom->byteCnt == 4)
            {
                mState = S_DATA;
                mRegAdr.b[1] = byte; 

                if(mWrite)
                {
                    mTimeoutId = add_alarm_in_us(mByteTimeoutUs, timeOutCb, (void*) this, true);
                }
                else
                {
                    // in read case we must turnaround the direction
                    aCom->uart->disableTx(false);

                    mRegTimeOutFlag = false;
                    mSeq->queueTask(mParaRWTaskId);
                    mRegTimeoutId = add_alarm_in_us(mByteTimeUs>>2, regTimeOutCb, (void*) this, true);
                }                    
            }
            else
            {
                mRegAdr.b[0] = byte;  

                mTimeoutId = add_alarm_in_us(mByteTimeoutUs, timeOutCb, (void*) this, true);
            }
            break;

        case S_DATA:
            if(aCom->byteCnt == 8)
            {
                mState = S_CRC;
            }

            if(mWrite)
            {
                cancel_alarm(mTimeoutId);
                mData.b[aCom->byteCnt - 5] = byte;
                mCrcCalc.dw = crcCalc(mCrcCalc.dw, byte);
                mTimeoutId = add_alarm_in_us(mByteTimeoutUs, timeOutCb, (void*) this, true);
            } 
            else
            {
                if(aCom->byteCnt < 7)
                {
                    // send byte 3 & 4 
                    aCom->uart->txChar(mData.b[aCom->byteCnt - 3]);
                    mCrcCalc.dw = crcCalc(mCrcCalc.dw, mData.b[aCom->byteCnt - 3]);
                }
                else
                {
                    // send byte 0 & 1
                    // if the register address is invalid send the inverted checksum
                    if(mInvalidRegAdr)
                        aCom->uart->txChar(~mCrcCalc.b[aCom->byteCnt - 7]);
                    else
                        aCom->uart->txChar(mCrcCalc.b[aCom->byteCnt - 7]);
                }
            }
            break;

        case S_CRC:
            if(mWrite)
            {
                cancel_alarm(mTimeoutId);
                mCrc.b[aCom->byteCnt - 9] = byte;
                if(aCom->byteCnt == 12)
                {
                    mState = S_IDLE;
                    if(mCrc.dw == mCrcCalc.dw)
                    {
                        mRegTimeOutFlag = false;
                        mSeq->queueTask(mParaRWTaskId);
                        mRegTimeoutId = add_alarm_in_us(mByteTimeUs>>2, regTimeOutCb, (void*) this, true);
                    }
                    aCom->byteCnt = 0;
                }
                else
                {
                    mTimeoutId = add_alarm_in_us(mByteTimeoutUs, timeOutCb, (void*) this, true);
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

                if(aCom->byteCnt < 11)
                {
                    // send byte 3 & 4
                    // if the register address is invalid send the inverted checksum
                    if(mInvalidRegAdr)
                        aCom->uart->txChar(~mCrcCalc.b[aCom->byteCnt - 7]);
                    else
                        aCom->uart->txChar(mCrcCalc.b[aCom->byteCnt - 7]);
                } 
            }
            break;

        case S_FWD:
            if(mWrite)
            {
                cancel_alarm(mTimeoutId);
                aCom->otherCom->uart->txChar(byte);
                if(aCom->byteCnt == 12)
                {
                    mState = S_IDLE;
                    aCom->byteCnt = 0;
                }
                else
                {
                    mTimeoutId = add_alarm_in_us(mByteTimeoutUs, timeOutCb, (void*) this, true);
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
                    // forword data until turnaround
                    if(aCom->byteCnt < 4)
                    {
                        cancel_alarm(mTimeoutId);
                        aCom->otherCom->uart->txChar(byte);
                        mTimeoutId = add_alarm_in_us(mByteTimeoutUs, timeOutCb, (void*) this, true);
                    }

                    if(aCom->byteCnt == 4)
                    {
                        cancel_alarm(mTimeoutId);
                        aCom->otherCom->uart->txChar(byte);
                        aCom->uart->disableTx(false);
                    }
                }
            }
            break;
        
        default:
            aCom->errCnt++;
            break;
        }
    }
    else
    {
        // this uart is currently the scondary uart
        if(aCom->byteCnt == 2 && byte == CInvalidAdr)
        {
            // massage abort detected, this means the recieved massage was for an device before this
            aCom->uart->disableTx(true);
            aCom->sec = false;
            aCom->byteCnt = 0;
        }

        if(aCom->byteCnt == 4 && !mWrite)
        {
            // turn around for read request needed
            aCom->uart->disableTx(true);
            mTimeoutId = add_alarm_in_us(mTurnAroundTimeout, timeOutCb, (void*) this, true);
        }

        if(aCom->byteCnt > 4 && !mWrite)
        {
            cancel_alarm(mTimeoutId);
            // forwarding data from readrequest to primary uart
            aCom->otherCom->uart->txChar(byte);
            if(aCom->byteCnt < 12)
            {
                mTimeoutId = add_alarm_in_us(mByteTimeoutUs, timeOutCb, (void*) this, true);
            }
        }

        if(aCom->byteCnt == 12)
        {
            // end of message
            aCom->byteCnt = 0;

            if(mWrite)
            {
                // before we disable the direction we to check if there is already the next
                // transfare is running
                if(mState == S_IDLE)
                {
                    aCom->uart->disableTx(true);
                    aCom->sec = false;
                };
            } 
            else
            {
                aCom->otherCom->uart->txChar(byte);
                aCom->sec = false;
            }
        }
    }  
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

int64_t GM_busSlave::timeOutCb(alarm_id_t id, void* aPObj)
{
    GM_busSlave* pObj = (GM_busSlave*) aPObj;

    if(pObj->mCom[0].sec)
    {
        pObj->mCom[1].errCnt++;
    }

    if(pObj->mCom[1].sec)
    {
        pObj->mCom[0].errCnt++;
    }

    pObj->mCom[0].sec = false;
    pObj->mCom[1].sec = false;
    pObj->mCom[0].byteCnt = 0;
    pObj->mCom[1].byteCnt = 0;
    pObj->mCom[0].uart->disableTx(true);
    pObj->mCom[1].uart->disableTx(true);

    pObj->mState = S_IDLE;

    pObj->mTimeoutId = 0;
    return 0;
}

int64_t GM_busSlave::regTimeOutCb(alarm_id_t id, void* aPObj)
{
    GM_busSlave* pObj = (GM_busSlave*) aPObj;

    pObj->mRegTimeOutFlag = true;

    pObj->mRegTimeoutId = 0;

    return 0;
}

void GM_busSlave::paraRW(void* aArg)
{
    GM_busSlave* pObj = (GM_busSlave*) aArg;

    if(pObj->mWrite)
    {
        cancel_alarm(pObj->mRegTimeoutId);
        pObj->mParaTable->setPara(pObj->mRegAdr.w, pObj->mData.dw);
    }
    else
    {
        // the first byte is directly loaded from send register to transmit engine
        // so we can push two bytes consecutivly into the send register
        pObj->mInvalidRegAdr = !pObj->mParaTable->getPara(pObj->mRegAdr.w, &pObj->mData.dw);

        if(!pObj->mRegTimeOutFlag)
        {
            cancel_alarm(pObj->mRegTimeoutId);
            // start answer to read request
            // rest is done in irq

            if(!pObj->mCom[0].sec)
            {
                pObj->mCom[0].uart->txChar(pObj->mData.b[0]);
                pObj->mCom[0].uart->txChar(pObj->mData.b[1]);
            }
            else
            {
                pObj->mCom[1].uart->txChar(pObj->mData.b[0]);
                pObj->mCom[1].uart->txChar(pObj->mData.b[1]);
            }
            pObj->mCrcCalc.dw = pObj->crcCalc(pObj->mCrcCalc.dw, pObj->mData.b[0]);
            pObj->mCrcCalc.dw = pObj->crcCalc(pObj->mCrcCalc.dw, pObj->mData.b[1]);
        }
    }
}
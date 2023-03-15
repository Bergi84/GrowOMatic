#include "gm_busSlave.h"

#include "rp_debug.h"

GM_busSlave::GM_busSlave() : GM_BusDefs()
{
    mState = S_IDLE;
    mInit = false;
}

void GM_busSlave::init(TUart *aUart0, TUart *aUart1, TParaTable *aParaTable, TSequencer* aSeq)
{
    if(!mInit)
    {
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
        gDebug.setPin(0); 
            
        switch (mState)
        {
        case S_IDLE:
            
            // both ports are in read mode, if we find a start deliminater
            // we can set the direction an start relaying
            if(byte == mSdW || byte == mSdR)
            {
                gDebug.setPin(1);
                mState = S_DEVADR;
                mWrite = (byte == mSdW);
                aCom->otherCom->sec = true;

                aCom->otherCom->uart->disableTx(false);
                aCom->otherCom->uart->txChar(byte);
                
                               
                mTimeoutId = add_alarm_in_us(mByteTimeoutUs, timeOutCb, (void*) this, true);
                gDebug.resetPin(1);
            }
            else
            {
                aCom->errCnt++;
            }
            break;

        case S_DEVADR:
            cancel_alarm(mTimeoutId);
            
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

                mTimeoutId = add_alarm_in_us(mByteTimeoutUs, timeOutCb, (void*) this, true);
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

                    mTimeoutId = add_alarm_in_us(mByteTimeoutUs, timeOutCb, (void*) this, true);

                    // calculate turnaround timout = transfaretime + 50% * bytetime
                    mTurnAroundTimeout = mByteTimeoutUs * 2 * (byte - 1) + ((mByteTimeUs * 3) >> 1);
                }
            }
            break;

        case S_REGADR:
            cancel_alarm(mTimeoutId);
            mCrcCalc.dw = crcCalc(mCrcCalc.dw, byte);

            if(mBroadCast)
                aCom->otherCom->uart->txChar(byte);

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
                    // this is done in idle task
                    mTimeoutId = 0;
                    mRegTimeOutFlag = false;
                    aCom->reqR = true;
                    aCom->uart->disableTx(false);
                    mSeq->queueTask(mParaRWTaskId);
                    mRegTimeoutId = add_alarm_in_us(mByteTimeUs >> 2, regTimeOutCb, (void*) this, true);
                }                    
            }
            else
            {
                mRegAdr.b[0] = byte;  

                mTimeoutId = add_alarm_in_us(mByteTimeoutUs, timeOutCb, (void*) this, true);
            }
            break;

        case S_DATA:
            cancel_alarm(mTimeoutId);

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
                mTimeoutId = add_alarm_in_us(mByteTimeoutUs, timeOutCb, (void*) this, true);
            } 
            else
            {
                if(aCom->byteCnt < 8)
                {
                    // send byte 1, 2 ,3 
                    aCom->uart->txChar(mData.b[aCom->byteCnt - 4]);
                    mTimeoutId = add_alarm_in_us(mByteTimeoutUs, timeOutCb, (void*) this, true);
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

                    mTimeoutId = add_alarm_in_us(mByteTimeoutUs, timeOutCb, (void*) this, true);
                }
            }
            break;

        case S_CRC:
            cancel_alarm(mTimeoutId);

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
                        mRegTimeOutFlag = false;
                        mSeq->queueTask(mParaRWTaskId);

                        // if the request is not written fast enougth the following
                        // datagramm can overwrite data
                        mRegTimeoutId = add_alarm_in_us(mByteTimeUs>>2, regTimeOutCb, (void*) this, true);
                    }
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
                    if(aCom->byteCnt < 12)
                    {
                        // send byte 1, 2, 3
                        // if the register address is invalid send the inverted checksum
                        if(mInvalidRegAdr)
                            aCom->uart->txChar(~mCrcCalc.b[aCom->byteCnt - 8]);
                        else
                            aCom->uart->txChar(mCrcCalc.b[aCom->byteCnt - 8]);

                        mTimeoutId = add_alarm_in_us(mByteTimeoutUs, timeOutCb, (void*) this, true);
                    } 
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
                    mTimeoutId = 0;
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
                    cancel_alarm(mTimeoutId);
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
                        cancel_alarm(mTimeoutId);
                        aCom->otherCom->uart->txChar(byte);
                        mTimeoutId = add_alarm_in_us(mByteTimeoutUs, timeOutCb, (void*) this, true);
                    }

                    if(aCom->byteCnt == 4)
                    {
                        cancel_alarm(mTimeoutId);
                        aCom->otherCom->uart->txChar(byte);
                        aCom->uart->disableTx(false);
                        mTimeoutId = add_alarm_in_us(mByteTimeoutUs, timeOutCb, (void*) this, true);
                    }
                }
            }
            break;
        
        default:
            aCom->errCnt++;
            break;
        }

        gDebug.resetPin(0);
    }
    else
    {
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
            cancel_alarm(mTimeoutId);
            aCom->uart->disableTx(true);
            mTimeoutId = add_alarm_in_us(mTurnAroundTimeout, timeOutCb, (void*) this, true);
        }

        if(aCom->byteCnt > 4 && !mWrite && mState == S_FWD)
        {
            cancel_alarm(mTimeoutId);
            // forwarding data from readrequest to primary uart
            aCom->otherCom->uart->txChar(byte);

            mTimeoutId = add_alarm_in_us(mByteTimeoutUs, timeOutCb, (void*) this, true);
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

int64_t GM_busSlave::timeOutCb(alarm_id_t id, void* aPObj)
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
    if(mBroadCast)
        while(1);

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

int64_t GM_busSlave::regTimeOutCb(alarm_id_t id, void* aPObj)
{
    GM_busSlave* pObj = (GM_busSlave*) aPObj;

    pObj->mRegTimeOutFlag = true;
    pObj->mRegTimeoutId = 0;

    return 0;
}

void GM_busSlave::paraRW(void* aArg)
{
    gDebug.setPin(2);

    GM_busSlave* pObj = (GM_busSlave*) aArg;

    if(pObj->mWrite)
    {
        uint32_t status = save_and_disable_interrupts();
        if(!pObj->mRegTimeOutFlag)
        {
            cancel_alarm(pObj->mRegTimeoutId);
            pObj->mRegTimeoutId = 0;
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
            cancel_alarm(pObj->mRegTimeoutId);
            pObj->mRegTimeoutId = 0;
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

            pObj->mTimeoutId = add_alarm_in_us(pObj->mByteTimeoutUs, timeOutCb, (void*) pObj, true);
        }
        else
        {
            // the system needs to long for the answare
            pObj->resetSlave();
        }
    }

    gDebug.resetPin(2);
}
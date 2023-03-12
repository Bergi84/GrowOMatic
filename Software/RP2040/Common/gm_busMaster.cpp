#include "gm_busMaster.h"
#include "hardware/sync.h"
#include "gm_device.h"


TBusCoordinator::TBusCoordinator() : GM_BusDefs()
{
    mInit = false;
    mDevListUpCb = 0;
}

void TBusCoordinator::init(TUart* aUart, TSequencer* aSeq)
{
    if(!mInit && mDevListUpCb)
    {
        mUart = aUart;
        mParaTable = 0;
        mSeq = aSeq;

        mErrTimeout = 0;
        mErrNoecho = 0;
        mErrWrongCrc = 0;

        mUart->config(mBaudRate, UP_NONE);

        // clear RX buffer
        uint8_t tmp;
        while(mUart->rxPending())
            mUart->rxChar(&tmp);

        mUart->disableFifo(true);
        mUart->disableTx(false);
        mUart->installRxCb(rxCb, (void*)this);

        mDevListLength = 0;    

        mReqQueue.rInd = 0;
        mReqQueue.wInd = 0;

        mByteTimeUs = (9999999 + mBaudRate)/mBaudRate;
        mByteTimeoutUs = (mByteTimeUs*18) >> 4;     // 112,5% of byte time

        mState = S_IDLE;
        mScanAktiv = false;

        mScanIntervallUs = 1000000;

        mSeq->addTask(mCoorTaskId, coorTask, this);
        add_repeating_timer_us(mScanIntervallUs, scanAlert, this, &mScanAlertTimer);

        mInit = true;
    }
}

void TBusCoordinator::init(TParaTable* aParaTable, TSequencer* aSeq)
{
    if(!mInit && mDevListUpCb)
    {
        mUart = 0;
        mParaTable = aParaTable;
        mSeq = aSeq;

        mDevListLength = 1;
        mParaTable->getPara(0, &mDeviceList[0]);

        mReqQueue.rInd = 0;
        mReqQueue.wInd = 0;

        mState = S_IDLE;
        mScanAktiv = false;

        mSeq->addTask(mCoorTaskId, coorTask, this);

        mInit = true;

        mDevListUpCb(mDevListUpCbArg, mDeviceList, mDevListLength);
    }
}

void TBusCoordinator::deinit()
{
    if(mInit)
    {
        if(mUart)
        {
            // todo: wait until S_IDLE before disable bus
            cancel_repeating_timer(&mScanAlertTimer);
            mUart->installRxCb(0, 0);
        }

        mUart = 0;
        mParaTable = 0;
        
        mSeq->delTask(mCoorTaskId);

        mInit = false;
    }
}

void TBusCoordinator::scan()
{
    if(!mScanAktiv)
    {
        mDeviceListUpdated = false;
        mScanAktiv = true;
        if(mDevListLength)
        {
            mScanIndex = mDevListLength-1;
        }
        else
        {
            mScanIndex = 0;
        }
        queueGetUid(mScanIndex, scanCb, this);
    }
}

void TBusCoordinator::scanCb(void* aArg, uint32_t* UId, errCode_T aStatus)
{
    TBusCoordinator* pObj = (TBusCoordinator*) aArg;
    if(aStatus == EC_SUCCESS)
    {
        if(((int32_t)pObj->mScanIndex) <= ((int32_t) pObj->mDevListLength) - 1)
        {
            // check existing device
            if(pObj->mDeviceList[pObj->mScanIndex] == *UId)
            {
                // last element is ok so the devices before also hav not changed
                pObj->mScanIndex++;
            }
            else
            {
                // there is a other device as stored, so we must reset the
                // device list and start from beginning
                pObj->mDevListLength = 0;
                pObj->mScanIndex = 0;
                pObj->mDeviceListUpdated = true;
            }
            pObj->queueGetUid(pObj->mScanIndex, pObj->scanCb, pObj);
        }
        else
        {
            // found new device
            pObj->mDeviceListUpdated = true;
            pObj->mDeviceList[pObj->mScanIndex] = *UId;
            pObj->mDevListLength = pObj->mScanIndex + 1;
            
            pObj->mScanIndex++;
            pObj->queueGetUid(pObj->mScanIndex, pObj->scanCb, pObj);
        }
    }
    else
    {
        if(((int32_t)pObj->mScanIndex) <= ((int32_t) pObj->mDevListLength) - 1)
        {
            // there is a missing device
            // rescan the hole bus
            pObj->mDevListLength = 0;
            pObj->mScanIndex = 0;
            pObj->mDeviceListUpdated = true;       

            pObj->queueGetUid(pObj->mScanIndex, pObj->scanCb, pObj);
        }
        else
        {
            // found the end of bus
            pObj->mScanAktiv = false;
            if(pObj->mDeviceListUpdated && pObj->mDevListUpCb)
                pObj->mDevListUpCb(pObj->mDevListUpCbArg, pObj->mDeviceList, pObj->mDevListLength);
        }
    }
}

uint8_t TBusCoordinator::getAdr(uint32_t aUid)
{
    if(!mInit)
        return CInvalidAdr;

    for(int i = 0; i < mDevListLength; i++)
    {
        if(mDeviceList[i] == aUid)
            return i;
    }
    return CInvalidAdr;
}

void TBusCoordinator::queueGetUid(uint8_t aAdr, void (*reqCb) (void*, uint32_t*, errCode_T), void* aArg)
{
    if(!mInit)
        return;

    uint32_t status = save_and_disable_interrupts();

    uint8_t newWInd = mReqQueue.wInd == GM_QUEUELEN - 1 ? 0 : mReqQueue.wInd + 1;
    if(newWInd == mReqQueue.rInd)
    {
        restore_interrupts(status);
        // todo: log error
        return;
    }

    reqRec_t* tmp =  &mReqQueue.buffer[mReqQueue.wInd];
    tmp->reqMode = RM_getUid;
    tmp->adr = aAdr;
    tmp->reqCb = reqCb;
    tmp->arg = aArg;
    tmp->write = false;
    tmp->paraAdr = 0;

    mReqQueue.wInd = newWInd;

    restore_interrupts(status);

    mSeq->queueTask(mCoorTaskId);
}

errCode_T TBusCoordinator::queueReadReq(reqAdr_t* aReqAdr, void (*reqCb) (void*, uint32_t*, errCode_T aStatus), void* aArg)
{
    if(!mInit)
        return EC_NOT_INIT;

    // check uid and if index exists
    if( (aReqAdr->devAdr >= mDevListLength) ||
        (aReqAdr->uid != mDeviceList[aReqAdr->devAdr]))
    {
        return EC_INVALID_DEVADR;
    }
    
    uint32_t status = save_and_disable_interrupts();

    uint8_t newWInd = mReqQueue.wInd == GM_QUEUELEN - 1 ? 0 : mReqQueue.wInd + 1;
    if(newWInd == mReqQueue.rInd)
    {
        // queue is full
        restore_interrupts(status);
        return EC_QUEUE_FULL;
    }

    reqRec_t* tmp =  &mReqQueue.buffer[mReqQueue.wInd];
    tmp->reqMode = RM_byAdr;
    tmp->uniqueId = aReqAdr->uid;
    tmp->adr = aReqAdr->devAdr;
    tmp->reqCb = reqCb;
    tmp->arg = aArg;
    tmp->write = false;
    tmp->paraAdr = aReqAdr->regAdr;

    mReqQueue.wInd = newWInd;

    restore_interrupts(status);

    mSeq->queueTask(mCoorTaskId);

    return EC_SUCCESS;
}

errCode_T TBusCoordinator::queueWriteReq(reqAdr_t* aReqAdr, uint32_t aVal, void (*reqCb) (void*, uint32_t*, errCode_T aStatus), void* aArg)
{
    if(!mInit)
        return EC_NOT_INIT;

    // check uid and if index exists
    if( (aReqAdr->devAdr >= mDevListLength) ||
        (aReqAdr->uid != mDeviceList[aReqAdr->devAdr]))
    {
        return EC_INVALID_DEVADR;
    }

    uint32_t status = save_and_disable_interrupts();

    uint8_t newWInd = mReqQueue.wInd == GM_QUEUELEN - 1 ? 0 : mReqQueue.wInd + 1;
    if(newWInd == mReqQueue.rInd)
    {
        restore_interrupts(status);
        return EC_QUEUE_FULL;
    }

    reqRec_t* tmp =  &mReqQueue.buffer[mReqQueue.wInd];
    tmp->reqMode = RM_byAdr;
    tmp->uniqueId = aReqAdr->uid;
    tmp->adr = aReqAdr->devAdr;
    tmp->reqCb = reqCb;
    tmp->arg = aArg;
    tmp->write = true;
    tmp->data = aVal;
    tmp->paraAdr = aReqAdr->regAdr;

    mReqQueue.wInd = newWInd;

    restore_interrupts(status);

    mSeq->queueTask(mCoorTaskId);

    return EC_SUCCESS;
}

void TBusCoordinator::coorTask(void* aArg)
{
    TBusCoordinator* pObj = (TBusCoordinator*) aArg;

    if(pObj->mState == S_READY)
    {  
        reqRec_t* tmp =  &pObj->mReqQueue.buffer[pObj->mReqQueue.rInd];
        if(tmp->write)
        {   
            // write request finsihed
            if(tmp->reqCb)
            {
                tmp->reqCb(tmp->arg, &tmp->data, EC_SUCCESS);
            }
            pObj->mReqQueue.rInd = pObj->mReqQueue.rInd == GM_QUEUELEN - 1 ? 0 : pObj->mReqQueue.rInd + 1;
            pObj->mState = S_IDLE;
        }
        else
        {
            cancel_alarm(pObj->mTimeoutId);
            // read request finished
            // check crc
            if(pObj->mCrcCalc == pObj->mCrc)
            {
                if(tmp->reqCb)
                {
                    tmp->reqCb(tmp->arg, &tmp->data, EC_SUCCESS);                
                }
                pObj->mReqQueue.rInd = pObj->mReqQueue.rInd == GM_QUEUELEN - 1 ? 0 : pObj->mReqQueue.rInd + 1;
                pObj->mRetryCnt = 0;
                pObj->mState = S_IDLE;
            }
            else
            {
                // wrong crc maybe a unique id request?
                if(tmp->reqMode == RM_getUid)
                {
                    // we must recalculate the crc with the recieved 
                    // unique id to detect if there was a transmission error
                    uint32_t locCrc = pObj->crcCalc(tmp->data, mSdR);   // SD
                    locCrc = pObj->crcCalc(locCrc, (uint8_t) 0);        // devAdr, always 0
                    locCrc = pObj->crcCalc(locCrc, (uint8_t) 0);        // regAdr[0]
                    locCrc = pObj->crcCalc(locCrc, (uint8_t) 0);        // regAdr[1]
                    locCrc = pObj->crcCalc(locCrc, tmp->dataB[0]);      // data[0]
                    locCrc = pObj->crcCalc(locCrc, tmp->dataB[1]);      // data[1]
                    locCrc = pObj->crcCalc(locCrc, tmp->dataB[2]);      // data[2]
                    locCrc = pObj->crcCalc(locCrc, tmp->dataB[3]);      // data[3]

                    if(locCrc == pObj->mCrc)
                    {
                        // uid recieved without errors
                        if(tmp->reqCb)
                        {
                            tmp->reqCb(tmp->arg, &tmp->data, EC_SUCCESS);                
                        }
                        pObj->mReqQueue.rInd = pObj->mReqQueue.rInd == GM_QUEUELEN - 1 ? 0 : pObj->mReqQueue.rInd + 1;
                        pObj->mRetryCnt = 0;
                        pObj->mState = S_IDLE;
                    }
                    else
                    {
                        pObj->mErrWrongCrc++;

                        // there was an transmission error, retry to get uid
                        if(pObj->mRetryCnt < GM_MAXRETRY)
                        {
                            pObj->mRetryCnt++;
                            pObj->mState = S_IDLE;
                        } else {
                            if(tmp->reqCb)
                            {
                                tmp->reqCb(tmp->arg, 0, EC_INVALID_UID);                
                            }
                            pObj->mReqQueue.rInd = pObj->mReqQueue.rInd == GM_QUEUELEN - 1 ? 0 : pObj->mReqQueue.rInd + 1;
                            pObj->mState = S_IDLE;

                            pObj->scan();
                        }
                    }
                }
                else
                {
                    if(pObj->mCrcCalc == ~pObj->mCrc)
                    {
                        // the slave send an inverted crc if the request belongs to an 
                        // not existing parameter
                        if(tmp->reqCb)
                        {
                            tmp->reqCb(tmp->arg, 0, EC_INVALID_REGADR);                
                        }
                        pObj->mReqQueue.rInd = pObj->mReqQueue.rInd == GM_QUEUELEN - 1 ? 0 : pObj->mReqQueue.rInd + 1;
                        pObj->mState = S_IDLE;
                    }
                    else
                    {
                        pObj->mErrWrongCrc++;

                        // transmission error or device order changed
                        if(pObj->mRetryCnt < GM_MAXRETRY)
                        {
                            pObj->mRetryCnt++;
                            pObj->mState = S_IDLE;
                        } else {
                            if(tmp->reqCb)
                            {
                                tmp->reqCb(tmp->arg, 0, EC_INVALID_DEVADR);                
                            }
                            pObj->mReqQueue.rInd = pObj->mReqQueue.rInd == GM_QUEUELEN - 1 ? 0 : pObj->mReqQueue.rInd + 1;
                            pObj->mState = S_IDLE;

                            pObj->scan();
                        }
                    }
                }
            }
        }
    }

    if(pObj->mState == S_TIMEOUT)
    {  
        reqRec_t* tmp =  &pObj->mReqQueue.buffer[pObj->mReqQueue.rInd];

        if(tmp->reqMode == RM_getUid)
        {
            // if the request was an get UID request we don't try it again
            // because the scan alart will it retry in fewe seconds again
            if(tmp->reqCb)
            {
                tmp->reqCb(tmp->arg, 0, EC_TIMEOUT);                
            }
            pObj->mReqQueue.rInd = pObj->mReqQueue.rInd == GM_QUEUELEN - 1 ? 0 : pObj->mReqQueue.rInd + 1;
            pObj->mState = S_IDLE;

        }
        else
        {
            pObj->mErrTimeout++;

            // transmission error or device order changed
            if(pObj->mRetryCnt < GM_MAXRETRY)
            {
                pObj->mRetryCnt++;
                pObj->mState = S_IDLE;
            } else {
                if(tmp->reqCb)
                {
                    tmp->reqCb(tmp->arg, 0, EC_TIMEOUT);                
                }
                pObj->mReqQueue.rInd = pObj->mReqQueue.rInd == GM_QUEUELEN - 1 ? 0 : pObj->mReqQueue.rInd + 1;
                pObj->mState = S_IDLE;

                pObj->scan();
            }            
        }
    };

    if(pObj->mState == S_ECHOERR)
    { 
        reqRec_t* tmp =  &pObj->mReqQueue.buffer[pObj->mReqQueue.rInd];

        pObj->mErrNoecho++;

        // bus is blocked or short circuite
        if(tmp->reqCb)
        {
            tmp->reqCb(tmp->arg, 0, EC_TIMEOUT);                
        }
        pObj->mReqQueue.rInd = pObj->mReqQueue.rInd == GM_QUEUELEN - 1 ? 0 : pObj->mReqQueue.rInd + 1;
        pObj->mState = S_IDLE;
    }

    if(pObj->mState == S_IDLE)
    {  
        // mUart == 0 means: this is virtual bus with the lokale device as slave
        // virtual bus uses only state S_IDLE
        if(pObj->mUart)
        {
            // idle look for new element in queue
            if(pObj->mReqQueue.rInd != pObj->mReqQueue.wInd)
            {
                // element found get address if needed
                reqRec_t* tmp =  &pObj->mReqQueue.buffer[pObj->mReqQueue.rInd];

                if(tmp->reqMode == RM_byUid)
                {
                    tmp->adr = pObj->getAdr(tmp->uniqueId);
                    if(tmp->adr == CInvalidAdr)
                    {
                        // invalid unique id, call callback function without value pointer
                        // to signal an error
                        if(tmp->reqCb)
                        {
                            tmp->reqCb(tmp->arg, 0, EC_INVALID_UID);
                            pObj->mReqQueue.rInd = pObj->mReqQueue.rInd == GM_QUEUELEN - 1 ? 0 : pObj->mReqQueue.rInd + 1;
                        }
                    }
                    else
                    {
                        pObj->sendReq();
                    }
                }
                else
                {
                    // RM_byAdr or RM_getUId
                    if(tmp->reqMode == RM_byAdr && tmp->uniqueId != pObj->mDeviceList[tmp->adr])
                    {
                        // invalid adress
                        tmp->reqCb(tmp->arg, 0, EC_INVALID_DEVADR);
                        pObj->mReqQueue.rInd = pObj->mReqQueue.rInd == GM_QUEUELEN - 1 ? 0 : pObj->mReqQueue.rInd + 1;
                    }
                    else
                    {
                        pObj->sendReq();
                    }
                }
            }
        }
        else
        {
            // look for new element in queue
            if(pObj->mReqQueue.rInd != pObj->mReqQueue.wInd)
            {
                // element found
                reqRec_t* tmp =  &pObj->mReqQueue.buffer[pObj->mReqQueue.rInd];

                switch(tmp->reqMode)
                {
                    case RM_getUid:
                    {
                        uint32_t locUid;
                        pObj->mParaTable->getPara(0, &locUid);
                        tmp->reqCb(tmp->arg, &locUid, EC_SUCCESS);
                    }
                    break;

                    case RM_byUid:
                        if(pObj->mDeviceList[0] == tmp->uniqueId)
                        {
                            if(tmp->write)
                            {
                                pObj->mParaTable->setPara(tmp->paraAdr, tmp->data);
                                tmp->reqCb(tmp->arg, &tmp->data, EC_SUCCESS);
                            }
                            else
                            {
                                uint32_t data;
                                if(pObj->mParaTable->getPara(tmp->paraAdr, &data))
                                {
                                    tmp->reqCb(tmp->arg, &data, EC_SUCCESS);
                                }
                                else
                                {
                                    tmp->reqCb(tmp->arg, 0, EC_INVALID_REGADR);
                                }
                            }
                        }
                        else
                        {
                            tmp->reqCb(tmp->arg, 0, EC_INVALID_UID);
                        }
                    break;
                

                case RM_byAdr:     
                    if(tmp->adr == 0 && pObj->mDeviceList[0] == tmp->uniqueId)
                    {
                        if(tmp->write)
                        {
                            pObj->mParaTable->setPara(tmp->paraAdr, tmp->data);
                            tmp->reqCb(tmp->arg, &tmp->data, EC_SUCCESS);
                        }
                        else
                        {
                            uint32_t data;
                            errCode_T ec = pObj->mParaTable->getPara(tmp->paraAdr, &data);
                            if(ec == EC_SUCCESS)
                            {
                                tmp->reqCb(tmp->arg, &data, EC_SUCCESS);
                            }
                            else
                            {
                                tmp->reqCb(tmp->arg, 0, EC_INVALID_REGADR);
                            }
                        }
                    }
                    else
                    {
                        tmp->reqCb(tmp->arg, 0, EC_INVALID_DEVADR);
                    }
                }
                pObj->mReqQueue.rInd = pObj->mReqQueue.rInd == GM_QUEUELEN - 1 ? 0 : pObj->mReqQueue.rInd + 1;
                pObj->mSeq->queueTask(pObj->mCoorTaskId);
            }
        }
    }
}

void TBusCoordinator::rxCb(void* aArg)
{
    TBusCoordinator* pObj = (TBusCoordinator*) aArg;

    reqRec_t* tmp =  &pObj->mReqQueue.buffer[pObj->mReqQueue.rInd];

    uint32_t rxCnt = 0;
    while(pObj->mUart->rxPending() && rxCnt < 4)
    {
        uint8_t byte;
        pObj->mUart->rxChar(&byte);
        pObj->mByteCntR++;
        
        if(!tmp->write && pObj->mByteCntR == 4)
        {
            // cancle timeout for echo reception
            cancel_alarm(pObj->mTimeoutId);
            pObj->mUart->disableTx(true);

            // start timout for answare
            pObj->mTimeoutId = add_alarm_in_us(pObj->mReqTimeoutUs, pObj->timeOutCb, (void*) pObj, true);
        }

        if(!tmp->write && pObj->mByteCntR > 4)
        {
            switch(pObj->mState)
            {
                case S_IDLE:
                case S_DEVADR: 
                case S_REGADR:
                    break;

                case S_DATA:
                    tmp->dataB[pObj->mByteCntR-5] = byte;
                    pObj->mCrcCalc = pObj->crcCalc(pObj->mCrcCalc, byte);
                    if(pObj->mByteCntR == 8)
                        pObj->mState = S_CRC;
                    break;

                case S_CRC:
                    pObj->mCrcB[pObj->mByteCntR-9] = byte;
                    if(pObj->mByteCntR == 12)
                    {
                        pObj->mState = S_READY;
                        pObj->mSeq->queueTask(pObj->mCoorTaskId);
                        pObj->mUart->disableTx(false);
                    }
                    break;

                case S_READY:
                    break;
            }
        }
        
        rxCnt++;
    }

    // wait for echo reception of last byte
    if(tmp->write && pObj->mByteCntW == 12 && pObj->mByteCntR == 12)
    {
        // cancle timeout for echo reception
        pObj->mState = S_READY;
        cancel_alarm(pObj->mTimeoutId);

        pObj->mSeq->queueTask(pObj->mCoorTaskId);
    }
    else
    {
        pObj->sendReq();
    }
}

void TBusCoordinator::sendReq()
{
    reqRec_t* tmp =  &mReqQueue.buffer[mReqQueue.rInd];

    while(mUart->txFree())
    {
        switch(mState)
        {
            case S_IDLE:
                if(tmp->write)
                {
                    mUart->txChar(mSdW);
                    mCrcCalc = crcCalc(mDeviceList[tmp->adr], mSdW);
                }
                else
                {
                    mUart->txChar(mSdR);
                    mCrcCalc = crcCalc(mDeviceList[tmp->adr], mSdR);
                    mReqTimeoutUs = mByteTimeoutUs * 2 * tmp->adr + (mByteTimeUs * 9);
                }

                if(tmp->write)
                    mTimeoutId = add_alarm_in_us(mByteTimeoutUs * 12, echoErrCb, (void*) this, true);
                else
                    mTimeoutId = add_alarm_in_us(mByteTimeoutUs * 4, echoErrCb, (void*) this, true);

                mState = S_DEVADR;
                mByteCntW = 1;
                mByteCntR = 0;
                break;

            case S_DEVADR:
                
                mUart->txChar(tmp->adr);
                mCrcCalc = crcCalc(mCrcCalc, (uint8_t) 0);

                mState = S_REGADR;  
                mByteCntW++;             
                break;

            case S_REGADR:
                {
                    uint8_t byte = tmp->paraAdrB[mByteCntW-2];
                    mUart->txChar(byte);
                    mCrcCalc = crcCalc(mCrcCalc, byte);

                    if(mByteCntW == 3)
                        mState = S_DATA; 
    
                    mByteCntW++;
                }      
                break;

            case S_DATA:
                if(tmp->write)
                {
                    uint8_t byte = tmp->dataB[mByteCntW-4];
                    mUart->txChar(byte);
                    mCrcCalc = crcCalc(mCrcCalc, byte);

                    if(mByteCntW == 7)
                        mState = S_CRC; 
 
                    mByteCntW++;
                }
                else
                {
                    return;
                }
                break;

            case S_CRC:
                if(tmp->write && mByteCntW < 12)
                {
                    uint8_t byte = mCrcCalcB[mByteCntW-8];
                    mUart->txChar(byte);

                    // switch to S_READY is donw after reception of last echo byte
                    // in rxCb task
 
                    mByteCntW++;
                }
                else
                {
                    return;
                }
                break;

            default:
                return;
        }
    }
}

int64_t TBusCoordinator::timeOutCb(alarm_id_t id, void* aArg)
{
    TBusCoordinator* pObj = (TBusCoordinator*) aArg;

    if(pObj->mState != S_READY)
    {
        pObj->mState = S_TIMEOUT;
        pObj->mUart->disableTx(false);
        pObj->mSeq->queueTask(pObj->mCoorTaskId);
    }

    return 0;
}

int64_t TBusCoordinator::echoErrCb(alarm_id_t id, void* aArg)
{
    TBusCoordinator* pObj = (TBusCoordinator*) aArg;

    if(pObj->mState != S_READY)
    {
        pObj->mState = S_ECHOERR;
        pObj->mUart->disableTx(false);
        pObj->mSeq->queueTask(pObj->mCoorTaskId);
    }

    return 0;
}

bool TBusCoordinator::scanAlert(repeating_timer_t *rt)
{
    TBusCoordinator* pObj = (TBusCoordinator*) rt->user_data;
    pObj->scan();

    return true;
}

void TBusCoordinator::installDeviceListUpdateCb(void (*mDeviceListChangedCb)(void* aArg, uint32_t* aUidList, uint32_t listLen), void* mDeviceListChangedCbArg)
{
    mDevListUpCb = mDeviceListChangedCb;
    mDevListUpCbArg = mDeviceListChangedCbArg;
}

GM_busMaster::GM_busMaster()
{
    mInit = false;
}

void GM_busMaster::init(TUart** aUartList, uint32_t aListLen, TSequencer* aSeq, TParaTable* aParaTable)
{
    if(!mInit)
    {
        mSeq = aSeq;

        mBusNo = aListLen + 1;

        mRootDev = 0;

        // bus 0 is a virtual bus with lokal device as slave
        mCbData[0].pObj = this;
        mCbData[0].busIndex = 0;
        mBusCoor[0].installDeviceListUpdateCb(mDevListUpCb, &mCbData[0]);
        mBusCoor[0].init(aParaTable, aSeq);

        for(int i = 1; i < aListLen + 1; i++)
        {
            mCbData[i].pObj = this;
            mCbData[i].busIndex = i;
            mBusCoor[i].installDeviceListUpdateCb(mDevListUpCb, &mCbData[i]);
            mBusCoor[i].init(aUartList[i - 1], aSeq);
        }

        for(int i = aListLen + 1; i < GM_MAXUARTS + 1; i++)
        {
            mCbData[i].pObj = 0;
            mCbData[i].busIndex = CInvalidBus;
        }

        mInit = true;
    }
}

void GM_busMaster::deinit()
{
    if(mInit)
    {
        for(int i = 0; i < GM_MAXUARTS + 1; i++)
        {
            if(mCbData[i].pObj)
                mBusCoor[i].deinit();
        }

        mInit = false;
    }
}

void GM_busMaster::mDevListUpCb(void* aArg, uint32_t* aUidList, uint32_t listLen)
{
    GM_busMaster* pObj = ((cbData_t*) aArg)->pObj;
    uint32_t bus = ((cbData_t*) aArg)->busIndex;

    uint32_t devFound[8] = {0};

    // search for existing devices
    GM_device* dev = pObj->mRootDev;
    GM_device* devBefore = 0;
    while(dev != 0)
    {
        bool found = false;
        for(int i = 0; i < listLen; i++)
        {
            if(dev->mUid == aUidList[i])
            {
                // device found
                devFound[i >> 5] |= 1 << (i & 0x1F);
                dev->updateAdr(bus, i);
                found = true;
                break;
            }
        }

        if(!found && dev->mBus == bus)
        {
            // device not found on same bus
            dev->updateAdr(CInvalidBus, CInvalidAdr);
            dev = devBefore;
        }
        
        devBefore = dev;
        dev = dev->mNext;
    }

    // add new devices to device list
    for(int i = 0; i < listLen; i++)
    {
        if(~devFound[i >> 5] & (1 << (i & 0x1F)))
        {
            // device not found, add a new one

            // todo: avoid new and delete with a pool of defined size
            GM_device* newDev = new GM_device(aUidList[i], pObj);
            newDev->updateAdr(bus, i);

            // add new device to list
            if(pObj->mRootDev == 0)
            {
                pObj->mRootDev = newDev;
            }
            else
            {
                devBefore->mNext = newDev;
            }
            devBefore = newDev;
        }
    }
}

void GM_busMaster::delDev(GM_device* aDev)
{
    if(aDev == mRootDev)
    {
        mRootDev = mRootDev->mNext;
        delete aDev;
    }
    else
    {
        GM_device* dev = mRootDev;
        while(dev->mNext != 0)
        {
            if(aDev == dev->mNext)
            {
                dev->mNext = dev->mNext->mNext;
                delete aDev;
                return;
            }
            dev = dev->mNext;
        }
    }
}

void GM_busMaster::devLost(uint8_t aBus, uint32_t aUid)
{
    for(int i = 0; i < mBusCoor[aBus].mDevListLength; i++)
    {
        if(mBusCoor[aBus].mDeviceList[i] == aUid)
        {
            mBusCoor[aBus].mDeviceList[i] = 0;
            mBusCoor[aBus].scan();
        }

    }
}

uint32_t GM_busMaster::getEpList(epType_t aEpType, TEpBase** aList, uint32_t aLen)
{
    if(!mInit)
        return 0;

    uint32_t epCnt = 0;

    GM_device* dev = mRootDev;
    while(dev)
    {
        TEpBase*  ep = dev->mEpList;
        while(ep)
        {
            if(ep->getType() == aEpType || aEpType == EPT_INVALID)
            {
                if(aList && epCnt < aLen)
                {
                    aList[epCnt] = ep;
                }
                epCnt++;
            }
            ep = ep->mNext;
        }
        dev->mNext;
    }
    return epCnt;
}

GM_device* GM_busMaster::findDev(uint8_t aBus, uint8_t aAdr)
{
    GM_device* dev = mRootDev;

    while(dev != 0)
    {
        if(dev->mBus == aBus && dev->mAdr == aAdr)
            return dev;

        dev = dev->mNext;
    }

    return 0;
}

GM_device* GM_busMaster::findDev(uint32_t aUid)
{
    GM_device* dev = mRootDev;

    while(dev != 0)
    {
        if(dev->mUid == aUid)
            return dev;

        dev = dev->mNext;
    }

    return 0;
}
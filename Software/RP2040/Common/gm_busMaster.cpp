#include "gm_busMaster.h"
#include "hardware/sync.h"


TBusCoordinator::TBusCoordinator() : GM_Bus()
{

}

void TBusCoordinator::init(TUart* aUart, TSequencer* aSeq)
{

    if(!mCrcTabInit)
        crcInitTab();

    mUart = aUart;
    mSeq = aSeq;

    mUart->config(mBaudRate, UART_PARITY_NONE);
    mUart->disableFifo(false);
    mUart->disableTx(false);
    mUart->installRxCb(rxCb, (void*)this);

    mDevListLength = 0;

    mReqQueue.rInd = 0;
    mReqQueue.wInd = 0;

    mByteTimeUs = (9999999 + mBaudRate)/mBaudRate;
    mByteTimeoutUs = (mByteTimeUs*18) >> 4;     // 112,5% of byte time

    mDeviceListChangedCb = 0;

    mState = S_IDLE;
    mScanAktiv = false;

    mScanIntervallUs = 5000000;

    mSeq->addTask(mCoorTaskId, coorTask, this);
    add_repeating_timer_us(mScanIntervallUs, scanAlert, this, &mScanAlertId);
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

void TBusCoordinator::scanCb(void* aArg, uint32_t* UId)
{
    TBusCoordinator* pObj = (TBusCoordinator*) aArg;
    if(UId)
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
        }
        else
        {
            // found the end of bus
            pObj->mScanAktiv = false;
            if(pObj->mDeviceListUpdated && pObj->mDeviceListChangedCb)
                pObj->mDeviceListChangedCb(pObj->mDeviceListChangedCbArg, pObj);
        }
    }
}

uint8_t TBusCoordinator::getAdr(uint32_t aUid)
{
    for(int i = 0; i < mDevListLength; i++)
    {
        if(mDeviceList[i] == aUid)
            return i;
    }
    return mInvalidAdr;
}

void TBusCoordinator::queueGetUid(uint8_t aAdr, void (*reqCb) (void*, uint32_t*), void* aArg)
{
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

void TBusCoordinator::queueReadReq(uint32_t aUId, uint16_t mParaAdr, void (*reqCb) (void*, uint32_t*), void* aArg)
{
    uint32_t status = save_and_disable_interrupts();

    uint8_t newWInd = mReqQueue.wInd == GM_QUEUELEN - 1 ? 0 : mReqQueue.wInd + 1;
    if(newWInd == mReqQueue.rInd)
    {
        restore_interrupts(status);
        // todo: log error
        return;
    }

    reqRec_t* tmp =  &mReqQueue.buffer[mReqQueue.wInd];
    tmp->reqMode = RM_byUid;
    tmp->uniqueId = aUId;
    tmp->reqCb = reqCb;
    tmp->arg = aArg;
    tmp->write = false;
    tmp->paraAdr = mParaAdr;

    mReqQueue.wInd = newWInd;

    restore_interrupts(status);

    mSeq->queueTask(mCoorTaskId);
}

void TBusCoordinator::queueWriteReq(uint32_t aUId, uint16_t mParaAdr, uint32_t aVal, void (*reqCb) (void*, uint32_t*), void* aArg)
{
    uint32_t status = save_and_disable_interrupts();

    uint8_t newWInd = mReqQueue.wInd == GM_QUEUELEN - 1 ? 0 : mReqQueue.wInd + 1;
    if(newWInd == mReqQueue.rInd)
    {
        restore_interrupts(status);
        // todo: log error
        return;
    }

    reqRec_t* tmp =  &mReqQueue.buffer[mReqQueue.wInd];
    tmp->reqMode = RM_byUid;
    tmp->uniqueId = aUId;
    tmp->reqCb = reqCb;
    tmp->arg = aArg;
    tmp->write = true;
    tmp->data = aVal;
    tmp->paraAdr = mParaAdr;

    mReqQueue.wInd = newWInd;

    restore_interrupts(status);

    mSeq->queueTask(mCoorTaskId);
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
                tmp->reqCb(tmp->arg, &tmp->data);
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
                    tmp->reqCb(tmp->arg, &tmp->data);                
                }
                pObj->mReqQueue.rInd = pObj->mReqQueue.rInd == GM_QUEUELEN - 1 ? 0 : pObj->mReqQueue.rInd + 1;
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
                    locCrc = pObj->crcCalc(locCrc, 0);                  // devAdr, always 0
                    locCrc = pObj->crcCalc(locCrc, 0);                  // regAdr[0]
                    locCrc = pObj->crcCalc(locCrc, 0);                  // regAdr[1]
                    locCrc = pObj->crcCalc(locCrc, tmp->dataB[0]);      // data[0]
                    locCrc = pObj->crcCalc(locCrc, tmp->dataB[1]);      // data[0]
                    locCrc = pObj->crcCalc(locCrc, tmp->dataB[2]);      // data[0]
                    locCrc = pObj->crcCalc(locCrc, tmp->dataB[3]);      // data[0]

                    if(locCrc == pObj->mCrc)
                    {
                        // uid recieved without errors
                        if(tmp->reqCb)
                        {
                            tmp->reqCb(tmp->arg, &tmp->data);                
                        }
                        pObj->mReqQueue.rInd = pObj->mReqQueue.rInd == GM_QUEUELEN - 1 ? 0 : pObj->mReqQueue.rInd + 1;
                        pObj->mState = S_IDLE;
                    }
                    else
                    {
                        // there was an transmission error, retry to get uid
                        pObj->mState = S_IDLE;
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
                            tmp->reqCb(tmp->arg, 0);                
                        }
                        pObj->mReqQueue.rInd = pObj->mReqQueue.rInd == GM_QUEUELEN - 1 ? 0 : pObj->mReqQueue.rInd + 1;
                        pObj->mState = S_IDLE;
                    }
                    else
                    {
                        // transmission error or device order changed
                        if(pObj->mRetryCnt < GM_MAXRETRY)
                        {
                            pObj->mRetryCnt++;
                            pObj->mState = S_IDLE;
                        } else {
                            if(tmp->reqCb)
                            {
                                tmp->reqCb(tmp->arg, 0);                
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
            // because the scan alart will is retry ist in fewe seconds again
            if(tmp->reqCb)
            {
                tmp->reqCb(tmp->arg, 0);                
            }
            pObj->mReqQueue.rInd = pObj->mReqQueue.rInd == GM_QUEUELEN - 1 ? 0 : pObj->mReqQueue.rInd + 1;
            pObj->mState = S_IDLE;

        }
        else
        {
            // transmission error or device order changed
            if(pObj->mRetryCnt < GM_MAXRETRY)
            {
                pObj->mRetryCnt++;
                pObj->mState = S_IDLE;
            } else {
                if(tmp->reqCb)
                {
                    tmp->reqCb(tmp->arg, 0);                
                }
                pObj->mReqQueue.rInd = pObj->mReqQueue.rInd == GM_QUEUELEN - 1 ? 0 : pObj->mReqQueue.rInd + 1;
                pObj->mState = S_IDLE;

                pObj->scan();
            }            
        }
    };

    if(pObj->mState == S_IDLE)
    {  
        pObj->mRetryCnt = 0;
        // idle look for new element in queue
        if(pObj->mReqQueue.rInd != pObj->mReqQueue.wInd)
        {
            // element found get address if needed
            reqRec_t* tmp =  &pObj->mReqQueue.buffer[pObj->mReqQueue.rInd];

            if(tmp->reqMode == RM_byUid)
            {
                tmp->adr = pObj->getAdr(tmp->uniqueId);
                if(tmp->adr == TBusCoordinator::mInvalidAdr)
                {
                    // invalid unique id, call callback function without value pointer
                    // to signal an error
                    if(tmp->reqCb)
                    {
                        tmp->reqCb(tmp->arg, 0);
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
                pObj->sendReq();
            }
        }
    }
    else
    {
        pObj->sendReq();
    }
}

void TBusCoordinator::rxCb(void* aArg)
{
    TBusCoordinator* pObj = (TBusCoordinator*) aArg;

    uint32_t rxCnt = 0;
    while(pObj->mUart->rxPending() && rxCnt < 4)
    {
        uint8_t byte;
        pObj->mUart->rxChar(&byte);
        pObj->mByteCntR++;

        reqRec_t* tmp =  &pObj->mReqQueue.buffer[pObj->mReqQueue.rInd];
        if(!tmp->write && pObj->mByteCntR == 4)
        {
            // start timout for answare
            pObj->mTimeoutId = add_alarm_in_us(pObj->mReqTimeoutUs, pObj->timeOutCb, (void*) pObj, true);
            pObj->mUart->disableTx(true);
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
                    tmp->dataB[pObj->mByteCntR-9] = byte;
                    pObj->mCrcCalc = pObj->crcCalc(pObj->mCrcCalc, byte);
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

                mState = S_DEVADR;
                mByteCntW = 1;
                mByteCntR = 0;
                break;

            case S_DEVADR:
                
                mUart->txChar(tmp->adr);
                mCrcCalc = crcCalc(mCrcCalc, 0);

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
                if(tmp->write)
                {
                    uint8_t byte = mCrcCalcB[mByteCntW-4];
                    mUart->txChar(byte);

                    if(mByteCntW == 13)
                    {
                        mState = S_READY; 
                        mSeq->queueTask(mCoorTaskId);
                    }
 
                    mByteCntW++;
                }
                else
                {
                    return;
                }
                break;

            case S_READY:
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

bool TBusCoordinator::scanAlert(repeating_timer_t *rt)
{
    TBusCoordinator* pObj = (TBusCoordinator*) rt->user_data;
    pObj->scan();

    return true;
}

void GM_busMaster::init(TUart** aUartList, uint32_t aListLen, TSequencer* aSeq)
{
    mSeq = aSeq;
    for(int i = 0; i < aListLen; i++)
    {
        mBus[i].init(aUartList[i], aSeq);
    }
}
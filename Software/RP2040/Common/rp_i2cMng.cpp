#include "rp_i2cMng.h"

void TI2CMng::init(i2c_inst_t* aI2C, TSequencer* aSeq, TTimerServer* aTS, uint8_t aSDAGpio, uint8_t aSCLGpio)
{
    mQueueStart = 0;
    mQueueEnd = 0;
    mWorkerTaskId = 0;

    mI2C = aI2C;
    mTS = aTS;
    mSeq = aSeq;

    gpio_set_function(aSDAGpio, GPIO_FUNC_I2C);
    gpio_set_function(aSCLGpio, GPIO_FUNC_I2C);

    mBaudRate = 100000;
    i2c_init(mI2C, mBaudRate);
    mByteTimeUs = (9 * 1000000 - 1 + mBaudRate)/mBaudRate;

    mIrqVeneer.init(this, irqHandler);

    uint8_t tmpSeqId;
    aSeq->addTask(tmpSeqId, setIrqHandler, this);
    aSeq->queueTask(tmpSeqId);   
    while(!aSeq->taskDone(tmpSeqId));
    aSeq->delTask(tmpSeqId);

    aSeq->addTask(mWorkerTaskId, workerTask, this);

    mTimeoutTimer = aTS->getTimer(timeOutCb, this);

    mState = S_IDLE;
}

void TI2CMng::setIrqHandler(void* aArg)
{
    TI2CMng* pObj = (TI2CMng*) aArg;
        
    if(pObj->mI2C == i2c0)
    {       
        irq_set_exclusive_handler(I2C0_IRQ, pObj->mIrqVeneer.getFunc());
        irq_set_enabled(I2C0_IRQ, true);
    }
    else
    {
        irq_set_exclusive_handler(I2C1_IRQ, pObj->mIrqVeneer.getFunc());
        irq_set_enabled(I2C1_IRQ, true);
    }

}

void TI2CMng::queueReq(TI2CReqRec* aReq)
{
    uint32_t status = save_and_disable_interrupts();

    if(mQueueStart == 0)
        mQueueStart = aReq;       
    else
        mQueueEnd->mNext = aReq;
    
    mQueueEnd = aReq;
    aReq->mNext = 0;

    restore_interrupts(status);

    mSeq->queueTask(mWorkerTaskId);
}

void TI2CMng::workerTask(void* aArg)
{
    TI2CMng* pObj = (TI2CMng*) aArg;

    if(pObj->mState == S_READY)
    {
        pObj->mQueueStart->mCb(pObj->mQueueStart->mCbArg, pObj->mQueueStart->mData, EC_SUCCESS);
        pObj->mQueueStart = pObj->mQueueStart->mNext;
        pObj->mState = S_IDLE;
    }

    if(pObj->mState == S_ERR)
    {
        pObj->mQueueStart->mCb(pObj->mQueueStart->mCbArg, pObj->mQueueStart->mData, pObj->mErr);
        pObj->mQueueStart = pObj->mQueueStart->mNext;
        pObj->mState = S_IDLE;
    }

    if(pObj->mState == S_RESET)
    {
        // should never occure, most likly the scl line stucks at low
        while(1);        
    }

    if(pObj->mState == S_IDLE)
    {
        pObj->mTxByteCnt = 0;
        pObj->mRxByteCnt = 0;
        if(pObj->mQueueStart->mRegAdrType == TI2CReqRec::RAL_NONE)
            pObj->mState = S_DATA;
        else
            pObj->mState = S_REGADR;


        // calculate timeout
        uint32_t bytCnt = pObj->mQueueStart->mDataLen;
        switch(pObj->mQueueStart->mRegAdrType )
        {
            case TI2CReqRec::RAL_8Bit:
                bytCnt += 1;
                break;
            case TI2CReqRec::RAL_16Bit:
                bytCnt += 2;
                break;
            case TI2CReqRec::RAL_32Bit:
                bytCnt += 4;
                break;
        }

        if(!pObj->mQueueStart->mWrite)
            bytCnt += 1;

        pObj->mTimeOut = false;
        pObj->mTimeoutTimer->setTimer((bytCnt + 2) * pObj->mByteTimeUs);

        // in this state the I2C unit is normaly disabled
        // so we can set the device address and enable the I2C
        // the enabling od I2C should trogger a TX Empty Interrupt
        pObj->mI2C->hw->intr_mask = I2C_IC_INTR_MASK_M_STOP_DET_BITS | I2C_IC_INTR_MASK_M_TX_EMPTY_BITS | I2C_IC_INTR_MASK_M_RX_FULL_BITS | I2C_IC_INTR_MASK_M_TX_ABRT_BITS;
        pObj->mI2C->hw->tar = pObj->mQueueStart->mDevAdr;
        pObj->mI2C->hw->enable = 1;
    }
}

void TI2CMng::irqHandler(void* aArg)
{
    TI2CMng* pObj = (TI2CMng*) aArg;
    TI2CReqRec* reqRec = pObj->mQueueStart;

    uint32_t abortReason = pObj->mI2C->hw->tx_abrt_source;
    if(pObj->mI2C->hw->clr_tx_abrt)
    {
        pObj->mTimeoutTimer->stopTimer();

        pObj->mState = S_ERR;

        if(!pObj->mTimeOut)
        {
            if(abortReason & I2C_IC_TX_ABRT_SOURCE_ABRT_USER_ABRT_BITS);
                pObj->mErr = EC_USER_ABORT;
            if(abortReason & I2C_IC_TX_ABRT_SOURCE_ARB_LOST_BITS)
                pObj->mErr = EC_ARB_ERR;
            if(abortReason & I2C_IC_TX_ABRT_SOURCE_ABRT_SBYTE_ACKDET_BITS)
                pObj->mErr = EC_UNKNOWEN;
            if(abortReason & I2C_IC_TX_ABRT_SOURCE_ABRT_HS_ACKDET_BITS)
                pObj->mErr = EC_UNKNOWEN;
            if(abortReason & I2C_IC_TX_ABRT_SOURCE_ABRT_TXDATA_NOACK_BITS)
                pObj->mErr = EC_INVALID_REGADR;
            if(abortReason & I2C_IC_TX_ABRT_SOURCE_ABRT_7B_ADDR_NOACK_BITS)
                pObj->mErr = EC_INVALID_DEVADR;
        }
        else
        {
            pObj->mErr = EC_TIMEOUT;
        }

        pObj->mI2C->hw->enable = 0;
        pObj->mI2C->hw->intr_mask = 0;

        // clear all interrupt flags
        volatile uint32_t sink;
        sink = pObj->mI2C->hw->clr_intr;

        pObj->mSeq->queueTask(pObj->mWorkerTaskId);
    }

    if(pObj->mI2C->hw->intr_stat & I2C_IC_INTR_STAT_R_RX_FULL_BITS)
    {
        // should never occure
        if(reqRec->mWrite)
            while(1);

        reqRec->mData[pObj->mRxByteCnt] = (uint8_t) pObj->mI2C->hw->data_cmd;
        pObj->mRxByteCnt++;
    }

    if(pObj->mI2C->hw->intr_stat & I2C_IC_INTR_STAT_R_TX_EMPTY_BITS)
    {
        bool done = false;

        while(pObj->mI2C->hw->txflr < 15 && !done)
        {
            switch(pObj->mState)
            {
                case S_REGADR:
                    pObj->mI2C->hw->data_cmd = reqRec->mRegAdr.A8B[pObj->mTxByteCnt];
                    pObj->mTxByteCnt++;

                    if( pObj->mTxByteCnt == 1 && reqRec->mRegAdrType == TI2CReqRec::RAL_8Bit ||
                        pObj->mTxByteCnt == 2 && reqRec->mRegAdrType == TI2CReqRec::RAL_16Bit ||
                        pObj->mTxByteCnt == 4 && reqRec->mRegAdrType == TI2CReqRec::RAL_32Bit )
                    {
                        pObj->mTxByteCnt = 0;
                        pObj->mState = S_DATA;
                    }
                    break;

                case S_DATA:
                    {
                        uint32_t ctrlBits = 0;

                        if(pObj->mTxByteCnt == 0 && reqRec->mRegAdrType != TI2CReqRec::RAL_NONE)
                        {
                            ctrlBits |= I2C_IC_DATA_CMD_RESTART_BITS;
                        }

                        if(!reqRec->mWrite)
                        {
                            ctrlBits |= I2C_IC_DATA_CMD_CMD_BITS;
                        }
            
                        if(pObj->mTxByteCnt == reqRec->mDataLen-1)
                        {
                            ctrlBits |= I2C_IC_DATA_CMD_STOP_LSB;

                            pObj->mI2C->hw->intr_mask = I2C_IC_INTR_MASK_M_STOP_DET_BITS | I2C_IC_INTR_MASK_M_RX_FULL_BITS | I2C_IC_INTR_MASK_M_TX_ABRT_BITS;
                            done = true;
                        }

                        pObj->mI2C->hw->data_cmd = ((uint32_t) reqRec->mData[pObj->mTxByteCnt]) | ctrlBits;
                        pObj->mTxByteCnt++;
                    }
                    break;

                default:
                    pObj->mI2C->hw->intr_mask = I2C_IC_INTR_MASK_M_STOP_DET_BITS | I2C_IC_INTR_MASK_M_RX_FULL_BITS | I2C_IC_INTR_MASK_M_TX_ABRT_BITS;
                    done = true;
            }
        }
    }

    if(pObj->mI2C->hw->clr_stop_det)
    {
        pObj->mTimeoutTimer->stopTimer();

        pObj->mState = S_READY;
        pObj->mSeq->queueTask(pObj->mWorkerTaskId);

        pObj->mI2C->hw->intr_mask = 0;
        pObj->mI2C->hw->enable = 0;

        // clear all interrupt flags
        volatile uint32_t sink;
        sink = pObj->mI2C->hw->clr_intr;
    }
}   

uint32_t TI2CMng::timeOutCb(void* aArg)
{
    // timeout call backs are excuted protected
    // so no further protection is needed

    TI2CMng* pObj = (TI2CMng*) aArg;

    uint32_t retVal;

    if(pObj->mTimeOut)
    {
        // timeout for abort

        if(pObj->mState != S_ERR)
        {
            // hardware recovery failed so we must reset the I2C hardware
            // this is done from idle task so we only disable the interrupts here
            pObj->mState = S_RESET;
            pObj->mI2C->hw->intr_mask = 0;
            if(pObj->mI2C == i2c0)
                irq_clear(I2C0_IRQ);
            else
                irq_clear(I2C1_IRQ);
            pObj->mSeq->queueTask(pObj->mWorkerTaskId);

            retVal = 0;
        }
    }
    else
    {
        // timeout for I2C transaction, try to abort

        pObj->mI2C->hw->intr_mask = I2C_IC_INTR_MASK_M_TX_ABRT_BITS;
        pObj->mI2C->hw->enable = 3;
        pObj->mTimeOut = true;
        retVal = pObj->mByteTimeUs * 2;
    }

    return retVal;
}
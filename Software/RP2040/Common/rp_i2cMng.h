#ifndef RP_I2CMNG_H_
#define RP_I2CMNG_H_

#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/platform.h"
#include "hardware/i2c.h"
#include "commonDefs.h"
#include "irqVeneer.h"

#ifndef I2C_QUEUELEN
#define I2C_QUEUELEN 8
#endif

#include "rp_timerServer.h"
#include "sequencer_armm0.h"

class TI2CMng;

class TI2CReqRec
{  
private:
    friend class TI2CMng;

    bool mWrite;
    uint8_t mDevAdr;
    typedef enum{
        RAL_NONE,
        RAL_8Bit,
        RAL_16Bit,
        RAL_32Bit
    } regAdrLen_t;
    regAdrLen_t mRegAdrType;
    union {
        uint8_t A8B[4];
        uint16_t A16B[2];
        uint32_t A32B;
    } mRegAdr;
    uint8_t* mData;
    uint32_t mDataLen;

    void (*mCb) (void* aArg, uint8_t* data, errCode_T aStatus);
    void* mCbArg;

    TI2CReqRec* mNext;

public:
    TI2CReqRec()
    {
        mCb = 0;
        mRegAdrType = RAL_NONE;
        mData = 0;
    }

    void setAdr(uint8_t aDevAdr)
    {
        mRegAdrType = RAL_NONE;
        mDevAdr = aDevAdr;
    }

    void setAdr(uint8_t aDevAdr, uint8_t aRegAdr)
    {
        mRegAdrType = RAL_8Bit;
        mRegAdr.A8B[0] = aRegAdr;
        mDevAdr = aDevAdr;
    }

    void setAdr(uint8_t aDevAdr, uint16_t aRegAdr)
    {
        mRegAdrType = RAL_16Bit;
        mRegAdr.A16B[0] = aRegAdr;
        mDevAdr = aDevAdr;
    }

    void setAdr(uint8_t aDevAdr, uint32_t aRegAdr)
    {
        mRegAdrType = RAL_32Bit;
        mRegAdr.A32B = aRegAdr;
        mDevAdr = aDevAdr;
    }

    void setData(bool aWrite, uint8_t* aData, uint32_t aDataLen)
    {
        mWrite = aWrite;
        mData = aData;
        mDataLen = aDataLen;
    }

    void setCb(void (*aCb) (void* aArg, uint8_t* data, errCode_T aStatus), void* aCbArg)
    {
        mCb = aCb;
        mCbArg = aCbArg;
    }
};

class TI2CMng
{
private:
    i2c_inst_t* mI2C;
    TTimerServer* mTS;
    TSequencer* mSeq;

    TI2CReqRec* mQueueStart;
    TI2CReqRec* mQueueEnd;

    irqVeneer_t mIrqVeneer;
    static void __time_critical_func(irqHandler)(void* aArg);
    static void setIrqHandler(void* aArg);

    typedef enum 
    {
        S_IDLE,
        S_REGADR,
        S_DATA,
        S_TIMEOUT,
        S_ERR,
        S_RESET,
        S_READY
    } state_e;
    state_e mState;
    uint32_t mTxByteCnt;
    uint32_t mRxByteCnt;

    uint32_t mBaudRate;
    uint32_t mByteTimeUs;

    static void workerTask(void* aArg);
    uint8_t mWorkerTaskId;

    static uint32_t timeOutCb(void* aArg);

    TTimer* mTimeoutTimer;
    bool mTimeOut;

public:
    void init(i2c_inst_t* aI2C, TSequencer* aSeq, TTimerServer* aTS, uint8_t aSDAGpio, uint8_t aSCLGpio);

    void queueReq(TI2CReqRec* aReq);
};

#endif /*RP_I2CMNG_H_*/
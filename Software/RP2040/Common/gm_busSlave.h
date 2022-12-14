
#ifndef GM_BUSSLAVE_H_
#define GM_BUSSLAVE_H_

#include "uart.h"
#include "paraTable.h"
#include "sequencer_armm0.h"
#include "gm_bus.h"

class GM_busSlave : public GM_Bus
{
private:
    typedef struct com_s { 
        TUart* uart;
        struct com_s* otherCom;
        uint32_t errCnt;
        uint32_t byteCnt;
        bool sec;
    } com_t;
    com_t mCom[2];

    static void rx0CbWrapper(void* aPObj);
    static void rx1CbWrapper(void* aPObj);
    static int64_t timeOutCb(alarm_id_t id, void* aPObj);
    static int64_t regTimeOutCb(alarm_id_t id, void* aPObj);

    void rxCb(com_t* aCom);

    uint32_t mCrcInitVal;
    union {
        uint32_t dw;
        uint8_t b[4];
    } mCrc;
    union {
        uint32_t dw;
        uint8_t b[4];
    } mCrcCalc;
    union {
        uint32_t dw;
        uint8_t b[4];
    } mData;
    union {
        uint16_t w;
        uint8_t b[2];
    } mRegAdr;
    bool mInvalidRegAdr;
    bool mWrite;

    typedef enum 
    {
        S_IDLE,
        S_DEVADR,
        S_REGADR,
        S_DATA,
        S_CRC,
        S_FWD
    } state_e;
    state_e mState;

    void (*mParaWCb)(void* aArg, uint16_t aRegAdr, uint32_t aData);
    bool (*mParaRCb)(void* aArg, uint16_t aRegAdr, uint32_t *aData);
    void* mParaCbArg;

    uint32_t mByteTimeUs;
    uint32_t mByteTimeoutUs;
    uint32_t mTurnAroundTimeout;

    alarm_id_t mTimeoutId;
    alarm_id_t mRegTimeoutId;

    TParaTable* mParaTable;
    TSequencer* mSeq;

    uint8_t mParaRWTaskId;
    bool mRegTimeOutFlag;

    static void paraRW(void* aArg);

public:
    GM_busSlave();
    void init(TUart *aUart0, TUart *aUart1, TParaTable *aParaTable, TSequencer* aSeq);
    void deinit();

    void setCrcInit(uint32_t aCrcInit) {mCrcInitVal = aCrcInit; };
};

#endif /*GM_BUSSLAVE_H_*/
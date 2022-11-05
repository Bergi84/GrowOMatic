
#ifndef GM_BUSSLAVE_H_
#define GM_BUSSLAVE_H_

#include "uart.h"

class GM_busSlave
{
private:
    static constexpr uint8_t mSdW = 0x68;
    static constexpr uint8_t mSdR = 0xA2;

    static constexpr uint32_t mBaudRate = 115200;

    static constexpr uint8_t mInvalidAdr = -1;

    static constexpr uint32_t mCrcPoly = 0xEDB88320;

    uint32_t mErrCnt0;
    uint32_t mErrCnt1;
    bool mDir0to1;
    bool mDir1to0;
    uint32_t mByteCnt0;
    uint32_t mByteCnt1;
    TUart* mUart0;
    TUart* mUart1;

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
    static void nextByteTimeOutCb(void* aPObj);

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
    bool (*mParaRCb)(void* aArg, uint16_t aRegAdr, uint32_t &aData);
    void* mParaCbArg;

    uint32_t byteTimeUs;
    uint32_t turnAroundTimeout;

    uint32_t crcTab[256];
    uint32_t crcCalc(uint32_t aCrc, uint8_t aByte);
    void crcInitTab();

public:
    GM_busSlave();
    void init(TUart *aUart0, TUart *aUart1);

    void setParaCb(void (*aParaWCb)(void*, uint16_t, uint32_t), bool (*aParaRCb)(void* , uint16_t , uint32_t &), void* aParaCbArg);
    void setCrcInit(uint32_t aCrcInit) {mCrcInitVal = aCrcInit; };
};

#endif /*GM_BUSSLAVE_H_*/
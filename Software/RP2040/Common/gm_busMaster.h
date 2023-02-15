#ifndef GM_BUSMASTER_H_
#define GM_BUSMASTER_H_

#include "uart.h"
#include "paraTable.h"
#include "sequencer_armm0.h"
#include "gm_busDefs.h"
#include "gm_epLib.h"

#ifndef GM_MAXSLAVES
#define GM_MAXSLAVES    64
#endif

#ifndef GM_MAXUARTS
#define GM_MAXUARTS    4
#endif

#ifndef GM_QUEUELEN
#define GM_QUEUELEN    16
#endif

#ifndef GM_MAXRETRY
#define GM_MAXRETRY    3
#endif


class TBusCoordinator : public GM_BusDefs
{
private:
    friend class GM_busMaster;

    TBusCoordinator();

    TUart* mUart;
    TParaTable* mParaTable;
    TSequencer* mSeq;

    bool mInit;

    uint32_t mDeviceList[GM_MAXSLAVES];
    uint8_t mDevListLength;

    uint32_t mByteTimeUs;
    uint32_t mByteTimeoutUs;
    uint32_t mReqTimeoutUs;

    uint64_t mScanIntervallUs;

    typedef enum
    {
        RM_getUid,
        RM_byAdr,
        RM_byUid
    } reqMode_t;

    typedef struct reqRec_s
    {
        uint32_t uniqueId;
        union {
            uint16_t paraAdr;
            uint8_t paraAdrB[2];
        };

        union {
            uint32_t data;
            uint8_t dataB[4];
        };

        bool write;
        reqMode_t reqMode;
        uint8_t adr;
        void* arg;
        void (*reqCb) (void* aArg, uint32_t* aVal, errCode_T aStatus);
    } reqRec_t;

    struct {
        uint8_t rInd;
        uint8_t wInd;
        reqRec_t buffer[GM_QUEUELEN];
    } mReqQueue;

    bool mScanAktiv;
    bool mDeviceListUpdated;

    void (*mDevListUpCb)(void* aArg, uint32_t* aUidList, uint32_t listLen);
    void* mDevListUpCbArg;

    uint8_t mScanIndex;
    static void scanCb(void* aArg, uint32_t* UId, errCode_T aStatus);
    void scan();

    void queueGetUid(uint8_t aAdr, void (*reqCb) (void*, uint32_t*, errCode_T), void* aArg);

    void sendReq();
    static void rxCb(void* aArg);

    union {
        uint32_t mCrc;
        uint8_t mCrcB[4];
    };
    union {
        uint32_t mCrcCalc;
        uint8_t mCrcCalcB[4];
    };
    typedef enum 
    {
        S_IDLE,
        S_DEVADR,
        S_REGADR,
        S_DATA,
        S_CRC,
        S_TIMEOUT,
        S_ECHOERR,
        S_READY
    } state_e;
    state_e mState;
    uint8_t mByteCntW;
    uint8_t mByteCntR;
    uint8_t mRetryCnt;

    alarm_id_t mTimeoutId;
    static int64_t timeOutCb(alarm_id_t id, void* aArg);
    static int64_t echoErrCb(alarm_id_t id, void* aArg);

    repeating_timer_t mScanAlertId;
    static bool scanAlert(repeating_timer_t *rt);

    static void coorTask(void* aArg);
    uint8_t mCoorTaskId;

    uint32_t mErrTimeout;
    uint32_t mErrNoecho;
    uint32_t mErrWrongCrc;

public:
    void init(TUart* aUart, TSequencer* aSeq);
    void deinit();
    bool isInit() { return mInit;   };

    // init of an virtual bus which holdes the loakel device as slave
    void init(TParaTable* aParaTable, TSequencer* aSeq);

    uint8_t getAdr(uint32_t aUid);

    errCode_T queueReadReq(reqAdr_t* aReqAdr, void (*reqCb) (void*, uint32_t*, errCode_T aStatus), void* aArg);
    errCode_T queueWriteReq(reqAdr_t* aReqAdr, uint32_t aVal, void (*reqCb) (void*, uint32_t*, errCode_T aStatus), void* aArg);
    void installDeviceListUpdateCb(void (*mDeviceListChangedCb)(void* aArg, uint32_t* aUidList, uint32_t listLen), void* mDeviceListChangedCbArg);
};

class GM_device;

class GM_busMaster
{
private:
    friend class GM_device;

    TBusCoordinator mBusCoor[GM_MAXUARTS + 1];  // bus 0 is the lokal device
    TSequencer* mSeq;

    typedef struct {
        GM_busMaster* pObj;
        uint32_t busIndex;
    } cbData_t;
    cbData_t mCbData[GM_MAXUARTS + 1];   // bus 0 is the lokal device

    static void mDevListUpCb(void* aArg, uint32_t* aUidList, uint32_t listLen);

    void delDev(GM_device* aDev);
    void devLost(uint8_t aBus, uint32_t aUid);

    GM_device* mRootDev;

    uint8_t mBusNo;

    bool mInit;

public:
    GM_busMaster();

    void init(TUart** aUartList, uint32_t aListLen, TSequencer* aSeq, TParaTable* aParaTable);
    void deinit();
    bool isInit() { return mInit;};

    inline errCode_T queueReadReq(reqAdr_t* aReqAdr, void (*reqCb) (void*, uint32_t*, errCode_T aStatus), void* aArg)
    {   return mBusCoor[aReqAdr->aBus].queueReadReq(aReqAdr, reqCb, aArg);  }

    inline errCode_T queueWriteReq(reqAdr_t* aReqAdr, uint32_t aVal, void (*reqCb) (void*, uint32_t*, errCode_T aStatus), void* aArg)
    {   return mBusCoor[aReqAdr->aBus].queueWriteReq(aReqAdr, aVal, reqCb, aArg);   }

    // returns written length, also when aList is null or aLen is 0
    // aLen ist the maximum length of aList, if aEpType == EPT_INVALID
    // function return all endpoints
    uint32_t getEpList(epType_t aEpType, TEpBase** aList, uint32_t aLen);
    GM_device* getDeviceLL() {  return mRootDev;}; 

    GM_device* findDev(uint8_t aBus, uint8_t aAdr);
    GM_device* findDev(uint32_t aUid);

    uint8_t getBusNo() { return mBusNo;};
};

#endif /*GM_BUSMASTER_H_*/
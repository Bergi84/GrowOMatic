#ifndef GM_DEVICE_H_
#define GM_DEVICE_H_

#include <stdint.h>
#include "gm_bus.h"

class GM_busMaster;
class GM_devUsedRec;

class TEpBase;

class GM_device {
private:
    friend class GM_busMaster;

    static constexpr uint32_t CEpScanIndDone = -1;

    GM_device();
    static const char* mDevNameList[];
    GM_busMaster* mBusMaster;
    uint32_t mUid;
    uint8_t mAdr;
    uint8_t mBus;
    devType_t mType;
    devStat_t mStat;

    uint32_t mEpScanInd;
    TEpBase* mEpList;
    TEpBase* mLastEp;

    GM_devUsedRec* mDevUsedList;
    
    static void epScanCb (void* aArg, uint32_t* aVal, errCode_T aStatus);
    void callStatUpCb();

    void startEpScan();

    inline void getDevAdr(reqAdr_t* aAdr) {aAdr->aBus = mBus; aAdr->devAdr = mAdr; aAdr->uid = mUid;};
public: 
    GM_device(uint32_t aUid, GM_busMaster* aBusMaster);

    void updateAdr(uint8_t aBus, uint8_t aAdr);

    const char* getDevName() {return mDevNameList[mType];}
    inline uint32_t getUid() {return mUid;}

    void regUsage(GM_devUsedRec* mDevUsedRec);
    void unregUsage(GM_devUsedRec* mDevUsedRec);

    errCode_T queueReadReq(uint16_t aRegAdr, void (*reqCb) (void*, uint32_t*, errCode_T aStatus), void* aArg);
    errCode_T queueWriteReq(uint16_t aRegAdr, uint32_t aVal, void (*reqCb) (void*, uint32_t*, errCode_T aStatus), void* aArg);

    GM_device* mNext;
} ;

class GM_devUsedRec {
public:
    GM_devUsedRec();

    inline void regUsage(GM_device* aDev)
        {   aDev->regUsage(this);   };

    inline void unregUsage()
        {   mDev->unregUsage(this); };

    inline void instStatusUpCb(void (*aUpCb) (void*, devStat_t), void* aCbArg)
        {   mUpCb = aUpCb;  mCbArg = aCbArg;    };
private:
    friend class GM_device;
    
    GM_devUsedRec* mNext;
    GM_device* mDev;
    void (*mUpCb) (void*, devStat_t);
    void* mCbArg;
};


#endif /* GM_DEVICE_H_*/
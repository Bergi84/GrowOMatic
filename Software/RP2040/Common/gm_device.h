#ifndef GM_DEVICE_H_
#define GM_DEVICE_H_

#include <stdint.h>
#include "gm_bus.h"

class GM_busMaster;
class GM_device;

class GM_devUsedRec {
public:
    GM_devUsedRec();

    void regUsage(GM_device* aDev);
    void unregUsage();
    void instStatusUpCb(void (*aUpCb) (void*, devStat_t), void* aCbArg);

private:
    friend class GM_device;
    
    GM_devUsedRec* mNext;
    GM_device* mDev;
    void (*mUpCb) (void*, devStat_t);
    void* mCbArg;
};

class TEpBase;

class GM_device {
private:
    friend class TEpBase;
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
public: 
    GM_device(uint32_t aUid, GM_busMaster* aBusMaster);

    void updateAdr(uint8_t aBus, uint8_t aAdr);

    const char* getDevName() {return mDevNameList[mType];}
    inline uint32_t getUid() {return mUid;}

    void regUsage(GM_devUsedRec* mDevUsedRec);
    void unregUsage(GM_devUsedRec* mDevUsedRec);

    GM_device* mNext;
} ;


#endif /* GM_DEVICE_H_*/
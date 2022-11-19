#ifndef GM_DEVICE_H_
#define GM_DEVICE_H_

#include <stdint.h>
#include "gm_bus.h"
#include "gm_epLib.h"

class GM_busMaster;

class GM_device {
private:
    friend class TEpBase;

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
    
    static void epScanCb (void* aArg, uint32_t* aVal, errCode_T aStatus);

    void startEpScan();
public: 
    GM_device(uint32_t aUid, GM_busMaster* aBusMaster);

    void updateAdr(uint8_t aBus, uint8_t aAdr);

    const char* getDevName() {return mDevNameList[mType];}
    inline uint32_t getUid() {return mUid;}

    GM_device* mNext;
} ;


#endif /* GM_DEVICE_H_*/
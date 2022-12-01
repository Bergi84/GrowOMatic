#include "gm_device.h"
#include "gm_epLib.h"
#include "gm_busMaster.h"

const char* GM_device::mDevNameList[] =
{
    [DT_INVALID] = "invalid",
    [DT_DUAL_LEVEL_SENSOR] = "dualLevelSensor",
    [DT_DUAL_VALVE_CON] = "dualValveCon"
};

GM_device::GM_device(uint32_t aUid, GM_busMaster* aBusMaster)
{
    mUid = aUid;
    GM_busMaster* mBusMaster = aBusMaster;
    devType_t mType = DT_INVALID;
    class TEpBase* epList = TEpBase::newEp(EPT_SYSTEM);
    mEpScanInd = 1;
    mBus = CInvalidBus;
    mAdr = CInvalidAdr;
    mNext = 0;
    mStat = DS_LOST;
}

void GM_device::updateAdr(uint8_t aBus, uint8_t aAdr)
{
    mBus = aBus;
    mAdr = aAdr;

    if(aBus == CInvalidBus && mStat != DS_LOST)
    {
        mStat = DS_LOST;
        if(mDevUsedList == 0)
        {
            mBusMaster->delDev(this);
            return;
        }
        else
        {
            callStatUpCb();
        }
    }

    if(aBus != CInvalidBus && mStat == DS_LOST)
    {
        if(mEpScanInd == CEpScanIndDone)
        {
            mStat = DS_AVAILABLE;
        }
        else
        {
            mStat = DS_NEW;
            startEpScan();
        }
        callStatUpCb();
    }
}

void GM_device::startEpScan()
{
    if(mType == DT_INVALID)
    {
        // first endpoint is always system
        ((TEpSystem*) mEpList)->getDevType(epScanCb, this);
    }
    else
    {
        reqAdr_t adr;
        adr.aBus = mBus;
        adr.devAdr = mAdr;
        adr.uid = mUid;
        adr.regAdr = CEpListBaseRegAdr + mEpScanInd - 1;

        mBusMaster->queueReadReq(&adr, epScanCb, this);
    }
}

void GM_device::epScanCb (void* aArg, uint32_t* aVal, errCode_T aStatus)
{
    GM_device* pObj = (GM_device*) aArg;

    switch(aStatus)
    {
        case EC_SUCCESS:
            // recieved scan value
            if(pObj->mType == DT_INVALID)
            {
                pObj->mType = devType_t (*aVal);
                pObj->startEpScan();
            }
            else
            {
                if(*aVal == EPT_INVALID)
                {
                    // scan done
                    pObj->mEpScanInd = CEpScanIndDone;
                    pObj->mStat = DS_AVAILABLE;
                    pObj->callStatUpCb();
                }
                else
                {
                    pObj->mLastEp->mNext = TEpBase::newEp(epType_t (*aVal) );
                    // ignore unkowen endpoints
                    if(pObj->mLastEp->mNext)
                    {
                        pObj->mEpScanInd++;
                        pObj->mLastEp = pObj->mLastEp->mNext;
                    }
                    pObj->startEpScan();
                }
            }
            break;

        case EC_TIMEOUT:
        case EC_INVALID_UID:
        case EC_INVALID_DEVADR:
        
            // lost device during scan
            pObj->mStat = DS_LOST;
            pObj->callStatUpCb();   
            break;

        case EC_QUEUE_FULL:
            // try again later

            // todo: install timer callback
            break;

        default:
            // scan done
            pObj->mEpScanInd = CEpScanIndDone;
            pObj->mStat = DS_AVAILABLE;
            pObj->callStatUpCb();
            break;
    
    }
}

void GM_device::callStatUpCb()
{
    GM_devUsedRec* akt = mDevUsedList;

    while(akt)
    {
        if(akt->mUpCb)
        {
            akt->mUpCb(akt->mCbArg, mStat);
        }
        akt = akt->mNext;
    }
}

void GM_device::regUsage(GM_devUsedRec* mDevUsedRec)
{
    if(mDevUsedRec->mDev != 0)
    {
        // unregister old device before register new one
        mDevUsedRec->mDev->unregUsage(mDevUsedRec);
    }
    mDevUsedRec->mDev = this;

    // insert at front of list becouse its faster
    mDevUsedRec->mNext = mDevUsedList;
    mDevUsedList = mDevUsedRec;
}

void GM_device::unregUsage(GM_devUsedRec* mDevUsedRec)
{
    if(mDevUsedList == mDevUsedRec)
    {
        mDevUsedList = mDevUsedRec->mNext;
        mDevUsedRec->mDev = 0;

        if(mDevUsedList == 0 && mStat == DS_LOST)
        {
            mBusMaster->delDev(this);
            return;
        }
    } 
    else
    {
        GM_devUsedRec* akt = mDevUsedList;

        while(akt->mNext)
        {
            if(akt->mNext == mDevUsedRec)
            {
                akt->mNext = akt->mNext->mNext;
                mDevUsedRec->mNext = 0;
                return;
            }
            akt = akt->mNext;
        }
    }
}

errCode_T GM_device::queueReadReq(uint16_t aRegAdr, void (*reqCb) (void*, uint32_t*, errCode_T aStatus), void* aArg)
{   
    reqAdr_t adr;
    getDevAdr(&adr); 
    adr.regAdr = aRegAdr;
    return mBusMaster->queueReadReq(&adr, reqCb, aArg);  
}

errCode_T GM_device::queueWriteReq(uint16_t aRegAdr, uint32_t aVal, void (*reqCb) (void*, uint32_t*, errCode_T aStatus), void* aArg)
{   
    reqAdr_t adr;
    getDevAdr(&adr); 
    adr.regAdr = aRegAdr;
    return mBusMaster->queueWriteReq(&adr, aVal, reqCb, aArg);   
}

GM_devUsedRec::GM_devUsedRec()
{
    mNext = 0;
    mDev = 0;
    mUpCb = 0;
}
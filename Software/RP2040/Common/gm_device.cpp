#include  "gm_device.h"


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

    if(mStat == DS_LOST)
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

        // todo: call status changed callbacks
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
        TBusCoordinator::reqAdr_t adr;
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
                pObj->mType = epType_t (*aVal);
                pObj->startEpScan();
            }
            else
            {
                if(*aVal == EPT_INVALID)
                {
                    // scan done
                    pObj->mEpScanInd = CEpScanIndDone;
                    mStat = DS_AVAILABLE;
                    // todo: call status changed callbacks
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

            // todo: call status changed callbacks            
            break;

        case EC_QUEUE_FULL:
            // try again later

            // todo: install timer callback
            break;

        default:
            // scan done
            pObj->mEpScanInd = CEpScanIndDone;
            mStat = DS_AVAILABLE;
            // todo: call status changed callbacks
            break;
    
    }
}
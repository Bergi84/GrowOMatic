#include "gm_device.h"
#include "gm_epLib.h"
#include "gm_busMaster.h"

GM_device::GM_device(uint32_t aUid, GM_busMaster* aBusMaster)
{
    mUid = aUid;
    mBusMaster = aBusMaster;
    mType = DT_INVALID;
    mLastEp = TEpBase::newEp(EPT_SYSTEM, this);
    mEpList = mLastEp;
    mEpScanInd = 0;
    mBus = CInvalidBus;
    mAdr = CInvalidAdr;
    mNext = 0;
    mStat = DS_LOST;
    mDevUsedList = 0;
    mErrUnkowenEp = 0;
}

void GM_device::updateAdr(uint8_t aBus, uint8_t aAdr)
{
    if(aBus == CInvalidBus && mStat != DS_LOST)
    {
        mBusMaster->devLost(mBus, mUid);
        mStat = DS_LOST;
        mBus = aBus;
        mAdr = aAdr;
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
    else
    {
        mBus = aBus;
        mAdr = aAdr;
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
        errCode_T ec = ((TEpSystem*) mEpList)->getDevType(epScanCb, this);
        checkErr(ec);
    }
    else
    {
        reqAdr_t adr;
        adr.aBus = mBus;
        adr.devAdr = mAdr;
        adr.uid = mUid;
        adr.regAdr = CEpListBaseRegAdr + mEpScanInd;

        errCode_T ec = mBusMaster->queueReadReq(&adr, epScanCb, this);
        checkErr(ec);
    }
}

void GM_device::reqNameCb (void* aArg, uint32_t* aVal, errCode_T aStatus)
{
    GM_device* pObj = (GM_device*) aArg;

    pObj->mWaitForName = false;
    pObj->startEpScan();
}

errCode_T GM_device::reqDevName(void (*aReqCb) (void*, uint32_t*, errCode_T), void* aArg)
{
    return ((TEpSystem*)mEpList)->reqDevName(&mDevName[0], aReqCb, aArg);
}

void GM_device::checkErr(errCode_T aEc)
{
    if(aEc != EC_SUCCESS)
    {
        updateAdr(CInvalidBus, CInvalidAdr);
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

                pObj->mWaitForName = true;
                errCode_T ec = pObj->reqDevName(reqNameCb, pObj);
                pObj->checkErr(ec);
            }
            else
            {
                if(pObj->mWaitForName)
                {
                    errCode_T ec;
                    if(pObj->mEpScanInd == 0)
                        ec = pObj->reqDevName(reqNameCb, pObj);
                    else
                        ec = pObj->mLastEp->reqEpName(reqNameCb, pObj);

                    pObj->checkErr(ec);
                }
                else
                {
                    epId_u epId;
                    epId.id = *aVal;

                    if(epId.type == EPT_INVALID)
                    {
                        // scan done
                        pObj->mEpScanInd = CEpScanIndDone;
                        pObj->mStat = DS_AVAILABLE;
                        pObj->callStatUpCb();
                    }
                    else
                    {
                        // new Endpoint, we always can insert the new Enpoints at the End
                        // because there is minimum the system endpoint in the list
                        pObj->mLastEp->mNext = TEpBase::newEp((epType_t) epId.type, pObj);
                        pObj->mEpScanInd++;
                        // ignore unkowen endpoints
                        if(pObj->mLastEp->mNext)
                        {
                            pObj->mLastEp = pObj->mLastEp->mNext;
                            pObj->mLastEp->mBaseAdr = epId.baseInd;

                            pObj->mWaitForName = true;
                            errCode_T ec = pObj->mLastEp->reqEpName(reqNameCb, pObj);
                            pObj->checkErr(ec);
                        }
                        else
                        {
                            pObj->mErrUnkowenEp++;
                            pObj->startEpScan();
                        }
                    }
                }
            }
            break;

        case EC_QUEUE_FULL:
            // try again later

            // todo: install timer callback
            break;

        case EC_INVALID_REGADR:
            // scan done
            pObj->mEpScanInd = CEpScanIndDone;
            pObj->mStat = DS_AVAILABLE;
            pObj->callStatUpCb();
            break;

//        case EC_TIMEOUT:
//        case EC_INVALID_UID:
//        case EC_INVALID_DEVADR:
        default:
            // lost device during scan
            pObj->updateAdr(CInvalidBus, CInvalidAdr); 
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

errCode_T GM_device::setDevName(char* aName, void (*aReqCb) (void*, uint32_t*, errCode_T), void* aArg)
{
    uint32_t i = 0;
    while(aName[i] != 0 && i < DEVICE_NAME_LEN) 
    {
        mDevName[i] = aName[i];
        i++;
    }
    mDevName[i] = 0;

    
    return ((TEpSystem*)mEpList)->setDevName(aName, aReqCb, aArg);
}

TEpBase* GM_device::findEp(uint16_t baseInd)
{
    TEpBase* aktEp = mEpList;
    while(aktEp)
    {
        if(aktEp->mBaseAdr == baseInd)
            return aktEp;

        aktEp = aktEp->mNext;
    }
    return 0;
}

GM_devUsedRec::GM_devUsedRec()
{
    mNext = 0;
    mDev = 0;
    mUpCb = 0;
}
#include "gm_epLib.h"
#include "gm_busMaster.h"
#include "gm_device.h"
#include "string.h"

TEpSystem::TEpSystem(GM_device* aDev)
{
    mType = cType;
    mParaList = cParaList;
    mParaListLen = cParaListLength;
    mTypeName = cTypeName;
    mPDev = aDev;
    strncpy(mName, cTypeName, EP_NAME_LEN + 1);
    mNext = 0;
}

void TEpSystem::setDevNameCb(void* aArg, uint32_t* aVal, errCode_T aStatus)
{
    TEpSystem* pObj = (TEpSystem*) aArg;

    if(aStatus != EC_SUCCESS)
        pObj->mNameCb(pObj->mNameCbArg, 0, aStatus);

    if(pObj->mNameInd < (EP_NAME_LEN + 4)/4)
    {
        uint32_t val = ((uint32_t) pObj->mDevName[0 + pObj->mNameInd*4]);
        val += (((uint32_t) pObj->mDevName[1 + pObj->mNameInd*4]) << 8);
        val += (((uint32_t) pObj->mDevName[2 + pObj->mNameInd*4]) << 16);
        val += (((uint32_t) pObj->mDevName[3 + pObj->mNameInd*4]) << 24);

        errCode_T ec = pObj->setPara(PARA_DEVNAME0 + pObj->mNameInd, val, pObj->setDevNameCb, (void*) pObj);
        pObj->mNameInd++;

        if(ec != EC_SUCCESS)
            pObj->mNameCb(pObj->mNameCbArg, 0, ec);    
    }
    else
    {
        pObj->mNameCb(pObj->mNameCbArg, (uint32_t*) &pObj->mDevName[0], aStatus);   
    }    
}

void TEpSystem::reqDevNameCb(void* aArg, uint32_t* aVal, errCode_T aStatus)
{
    TEpSystem* pObj = (TEpSystem*) aArg;

    if(aStatus != EC_SUCCESS)
        pObj->mNameCb(pObj->mNameCbArg, 0, aStatus);

    pObj->mDevName[0 + pObj->mNameInd*4] = (uint8_t) (*aVal);
    pObj->mDevName[1 + pObj->mNameInd*4] = (uint8_t) ((*aVal) >> 8);
    pObj->mDevName[2 + pObj->mNameInd*4] = (uint8_t) ((*aVal) >> 16);
    pObj->mDevName[3 + pObj->mNameInd*4] = (uint8_t) ((*aVal) >> 24);

    pObj->mNameInd++;

    if(pObj->mNameInd < (EP_NAME_LEN + 4)/4)
    {
        errCode_T ec = pObj->reqPara(PARA_DEVNAME0 + pObj->mNameInd, pObj->reqDevNameCb, (void*) pObj);

        if(ec != EC_SUCCESS)
            pObj->mNameCb(pObj->mNameCbArg, 0, ec);  
    }
    else
    {
        pObj->mNameCb(pObj->mNameCbArg, (uint32_t*) &pObj->mDevName[0], aStatus);  
    }
}

errCode_T TEpSystem::setDevName(char* aName, void (*aReqCb) (void*, uint32_t*, errCode_T), void* aArg) 
{
    mDevName = aName;
    mNameInd = 0;
    mNameCb = aReqCb;
    mNameCbArg = aArg;

    if(mType == EPT_SYSTEM)
        return EC_PROTECTED;

    uint32_t val = ((uint32_t) mDevName[0 + mNameInd*4]);
    val += (((uint32_t) mDevName[1 + mNameInd*4]) << 8);
    val += (((uint32_t) mDevName[2 + mNameInd*4]) << 16);
    val += (((uint32_t) mDevName[3 + mNameInd*4]) << 24);

    errCode_T ec = setPara(PARA_DEVNAME0 + mNameInd, val, setDevNameCb, (void*) this);
    mNameInd++;

    return ec;
}

errCode_T TEpSystem::reqDevName(char* aName, void (*aReqCb) (void*, uint32_t*, errCode_T), void* aArg)
{
    mDevName = aName;
    mNameInd = 0;
    mNameCb = aReqCb;
    mNameCbArg = aArg;

    errCode_T ec = reqPara(PARA_DEVNAME0 + mNameInd, reqDevNameCb, (void*) this);

    return ec;
}


TEpBus::TEpBus(GM_device* aDev)
{
    mType = cType;
    mParaList = cParaList;
    mParaListLen = cParaListLength;
    mTypeName = cTypeName;
    mPDev = aDev;
    mName[0] = 0;
    mNext = 0;
}

TEpBase* TEpBase::newEp(epType_t aEpType, GM_device* aDev)
{
    switch(aEpType)
    {
        case EPT_SYSTEM:
            return new TEpSystem(aDev);

        case EPT_BUS:
            return new TEpBus(aDev);

        default:
            return 0;
    }
}

TEpBase::TEpBase()
{
    mType = EPT_INVALID;
    mParaList = 0;
    mParaListLen = 0;
    mTypeName = cInvalidName;
    mBaseAdr = 0;
    mNext = 0;
}

void TEpBase::regUsage(GM_devUsedRec* mDevUsedRec)
{
    mPDev->regUsage(mDevUsedRec);
}

void TEpBase::unregUsage(GM_devUsedRec* mDevUsedRec)
{
    mPDev->unregUsage(mDevUsedRec);
}

void TEpBase::setEpNameCb(void* aArg, uint32_t* aVal, errCode_T aStatus)
{
    TEpBase* pObj = (TEpBase*) aArg;

    if(aStatus != EC_SUCCESS)
        pObj->mNameCb(pObj->mNameCbArg, 0, aStatus);

    if(pObj->mNameInd < (EP_NAME_LEN + 4)/4)
    {
        uint32_t val = ((uint32_t) pObj->mName[0 + pObj->mNameInd*4]);
        val += (((uint32_t) pObj->mName[1 + pObj->mNameInd*4]) << 8);
        val += (((uint32_t) pObj->mName[2 + pObj->mNameInd*4]) << 16);
        val += (((uint32_t) pObj->mName[3 + pObj->mNameInd*4]) << 24);

        errCode_T ec = pObj->setPara(pObj->mParaListLen + pObj->mNameInd, val, pObj->setEpNameCb, (void*) pObj);
        pObj->mNameInd++;

        if(ec != EC_SUCCESS)
            pObj->mNameCb(pObj->mNameCbArg, 0, ec);    
    }
    else
    {
        pObj->mNameCb(pObj->mNameCbArg, (uint32_t*) &pObj->mName[0], aStatus);   
    }    
}

void TEpBase::reqEpNameCb(void* aArg, uint32_t* aVal, errCode_T aStatus)
{
    TEpBase* pObj = (TEpBase*) aArg;

    if(aStatus != EC_SUCCESS)
        pObj->mNameCb(pObj->mNameCbArg, 0, aStatus);

    pObj->mName[0 + pObj->mNameInd*4] = (uint8_t) (*aVal);
    pObj->mName[1 + pObj->mNameInd*4] = (uint8_t) ((*aVal) >> 8);
    pObj->mName[2 + pObj->mNameInd*4] = (uint8_t) ((*aVal) >> 16);
    pObj->mName[3 + pObj->mNameInd*4] = (uint8_t) ((*aVal) >> 24);

    pObj->mNameInd++;

    if(pObj->mNameInd < (EP_NAME_LEN + 4)/4)
    {
        errCode_T ec = pObj->reqPara(pObj->mParaListLen + pObj->mNameInd, pObj->reqEpNameCb, (void*) pObj);

        if(ec != EC_SUCCESS)
            pObj->mNameCb(pObj->mNameCbArg, 0, ec);  
    }
    else
    {
        pObj->mNameCb(pObj->mNameCbArg, (uint32_t*) &pObj->mName[0], aStatus);  
    }
}

errCode_T TEpBase::setEpName(char* aName, void (*aReqCb) (void*, uint32_t*, errCode_T), void* aArg) 
{
    mNameInd = 0;
    mNameCb = aReqCb;
    mNameCbArg = aArg;

    if(mType == EPT_SYSTEM)
        return EC_PROTECTED;

    uint32_t i = 0;
    while(aName[i] != 0 && i < EP_NAME_LEN) 
    {
        mName[i] = aName[i];
        i++;
    }
    mName[i] = 0;

    uint32_t val = ((uint32_t) mName[0 + mNameInd*4]);
    val += (((uint32_t) mName[1 + mNameInd*4]) << 8);
    val += (((uint32_t) mName[2 + mNameInd*4]) << 16);
    val += (((uint32_t) mName[3 + mNameInd*4]) << 24);

    errCode_T ec = setPara(mParaListLen + mNameInd, val, setEpNameCb, (void*) this);
    mNameInd++;

    return ec;
}

errCode_T TEpBase::reqEpName(void (*aReqCb) (void*, uint32_t*, errCode_T), void* aArg)
{
    mNameInd = 0;
    mNameCb = aReqCb;
    mNameCbArg = aArg;

    if(mType == EPT_SYSTEM)
        return EC_PROTECTED;

    errCode_T ec = reqPara(mParaListLen + mNameInd, reqEpNameCb, (void*) this);

    return ec;
}

#include "gm_epLib.h"
#include "gm_busMaster.h"
#include "gm_device.h"

TEpSystem::TEpSystem(GM_device* aDev)
{
    mType = cType;
    mParaList = cParaList;
    mParaListLen = cParaListLength;
    mTypeName = cTypeName;
    mPDev = aDev;
    mNext = 0;
}

TEpBus::TEpBus(GM_device* aDev)
{
    mType = cType;
    mParaList = cParaList;
    mParaListLen = cParaListLength;
    mTypeName = cTypeName;
    mPDev = aDev;
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

void TEpBase::setEpName(char* aName) 
{
    // todo: send new endpoint name to endpoint
    uint32_t i = 0;
    while(aName[i] != 0 && i < EP_NAME_LEN) 
    {
        mName[i] = aName[i];
        i++;
    }
    mName[i] = 0;
}

void TEpBase::reqEpName()
{
    // todo: implement
}

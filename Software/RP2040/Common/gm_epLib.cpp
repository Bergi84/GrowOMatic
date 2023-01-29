#include "gm_epLib.h"
#include "gm_busMaster.h"
#include "gm_device.h"

TEpSystem::TEpSystem()
{
    mType = cType;
    mParaList = cParaList;
    mParaListLen = cParaListLength;
    mTypeName = cTypeName;
    mNext = 0;
}

TEpBus::TEpBus()
{
    mType = cType;
    mParaList = cParaList;
    mParaListLen = cParaListLength;
    mTypeName = cTypeName;
    mNext = 0;
}

TEpBase* TEpBase::newEp(epType_t aEpType)
{
    switch(aEpType)
    {
        case EPT_SYSTEM:
            return new TEpSystem();

        case EPT_BUS:
            return new TEpBus();

        default:
            return 0;
    }
}

const char TEpBase::cInvalidName[] = "unkowen";

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

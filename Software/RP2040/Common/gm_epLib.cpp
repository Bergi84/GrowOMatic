#include "gm_epLib.h"
#include "gm_busMaster.h"
#include "gm_device.h"

const paraDef_t TEpSystem::cParaList[] =
{
    [PARA_UID] = {paraDef_t::PARA_FLAG_R, "uniqueId"},
    [PARA_TYPE] = {paraDef_t::PARA_FLAG_R, "deviceType"},
    [PARA_FWVERSION] = {paraDef_t::PARA_FLAG_R, "fwVersion"}
};

const uint32_t TEpSystem::cParaListLength = sizeof(cParaList)/ sizeof(paraDef_t);

const char TEpSystem::cName[] = "system";

TEpSystem::TEpSystem()
{
    mType = cType;
    mParaList = cParaList;
    mParaListLen = cParaListLength;
    mEpName = cName;
    mNext = 0;
}

TEpBase* TEpBase::newEp(epType_t aEpType)
{
    switch(aEpType)
    {
        case EPT_SYSTEM:
            return new TEpSystem();

        default:
            return 0;
    }
}

void TEpBase::regUsage(GM_devUsedRec* mDevUsedRec)
{
    mPDev->regUsage(mDevUsedRec);
}

void TEpBase::unregUsage(GM_devUsedRec* mDevUsedRec)
{
    mPDev->unregUsage(mDevUsedRec);
}
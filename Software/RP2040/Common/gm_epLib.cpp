#include "gm_epLib.h"
#include "gm_busMaster.h"
#include "gm_device.h"

const paraDef_t TEpSystem::cParaList[] =
{
    [PARA_UID] =        {paraDef_t::PARA_FLAG_R, "uniqueId"},
    [PARA_TYPE] =       {paraDef_t::PARA_FLAG_R, "deviceType"},
    [PARA_FWVERSION] =  {paraDef_t::PARA_FLAG_R, "fwVersion"},
    [PARA_SAVE] =       {paraDef_t::PARA_FLAG_W, "savePara"},
    [PARA_START] =      {paraDef_t::PARA_FLAG_W, "start"},

};

const uint32_t TEpSystem::cParaListLength = sizeof(cParaList)/ sizeof(paraDef_t);

const char TEpSystem::cTypeName[] = "system";

TEpSystem::TEpSystem()
{
    mType = cType;
    mParaList = cParaList;
    mParaListLen = cParaListLength;
    mTypeName = cTypeName;
    mNext = 0;
}

const paraDef_t TEpBus::cParaList[] =
{
    [PARA_MASTEREN] =        {paraDef_t::PARA_FLAG_R, "masterEn"},
};

const uint32_t TEpBus::cParaListLength = sizeof(cParaList)/ sizeof(paraDef_t);

const char TEpBus::cTypeName[] = "bus";

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
    mNext = 0;
}

void TEpBase::generateName()
{
    // find unused name
    const char* newName = getTypeName();
    uint32_t nameNo = 0;
    
    if(mPDev->mEpList)
    {
        TEpBase* tmp = mPDev->mEpList;
        TEpBase* stop = 0;   // should hold the last

        while(tmp != stop)
        { 

            char* tmpName = tmp->getEpName();
            uint32_t k = 0;

            // compare names
            while(tmpName[k] == newName[k] && tmpName[k] != 0 && newName[k] != 0)
            {
                k++;
            }
            if(newName[k] == 0)
            {
                // device name is until end of newName the same
                if(nameNo/10 + '0' == tmpName[k] && nameNo%10 + '0' == tmpName[k+1])
                {
                    // device name is the same
                    nameNo++;
                    stop = tmp;
                }

            }

            // iterate to next item
            if(tmp->mNext)
                tmp = tmp->mNext;
            else
                tmp = mPDev->mEpList;

            // init stop after first loop
            if(stop == 0)
                stop = mPDev->mEpList;   // should hold the last
        }
    }

    // set name 
    uint32_t i = 0;
    while(i < DEVICE_NAME_LEN - 2 && newName[i] != 0)
    {
        mName[i] = newName[i];
        i++;
    }
    mName[i] = nameNo/10 + '0';
    i++;
    mName[i] = nameNo%10 + '0';
    i++;
    mName[i] = 0;
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
    uint32_t i = 0;
    while(aName[i] != 0 && i < EP_NAME_LEN) 
    {
        mName[i] = aName[i];
        i++;
    }
    mName[i] = 0;
}
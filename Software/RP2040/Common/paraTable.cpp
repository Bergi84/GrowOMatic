#include "paraTable.h"
#include "gm_epLib.h"
#include "pico/bootrom.h"
#include "hardware/watchdog.h"

TParaTable::TParaTable() : 
// init system parameter list
mSysPara( (paraRec_t[mSysParaLen]) {
    [PARA_UID] =          {.para = 0,          .pFAccessCb = 0, .cbArg = 0, .flags = PARA_FLAG_R},
    [PARA_TYPE] =         {.para = 0,          .pFAccessCb = 0, .cbArg = 0, .flags = PARA_FLAG_R},
    [PARA_FWVERSION] =    {.para = VER_COMBO,  .pFAccessCb = 0, .cbArg = 0, .flags = PARA_FLAG_R},
    [PARA_SAVE] =         {.para = 0,          .pFAccessCb = paraSaveCb, .cbArg = this, .flags = PARA_FLAG_W | PARA_FLAG_FW},
    [PARA_START] =        {.para = 0,          .pFAccessCb = paraStartCb, .cbArg = this, .flags = PARA_FLAG_W | PARA_FLAG_FW},
    }),

// init Endpoint List Endpoint 
mEpListEndpoint( (endpoint_t) {
    { { 
        .startIndex = CEpListBaseRegAdr,
        .type = (uint16_t)EPT_EPLIST    
    } }, 
    .length = 1, 
    .para = mEpListPara,
    .next = 0
}),
// init system endpoint
mSysEndpoint( (endpoint_t) { 
    { { 
        .startIndex = CSystemBaseRegAdr,
        .type = (uint16_t)EPT_SYSTEM    
    } }, 
    .length = sizeof(mSysPara)/sizeof(paraRec_t), 
    .para = mSysPara,
    .next = &mEpListEndpoint
})
{
    for(int i = 0; i < PT_MAXENDPOINTS; i++)
    {
        mEpListPara[i] = {.para = EPT_INVALID, .pFAccessCb = 0, .cbArg = 0, .flags = PARA_FLAG_R};
    }
}

void TParaTable::init(uint32_t aUniqueId, devType_t aDevType, TStorage* aStorage)
{
    mSysPara[PARA_UID].para = aUniqueId;
    mSysPara[PARA_TYPE].para = (uint32_t) aDevType;
    mStorage = aStorage;
}

void TParaTable::addEndpoint(endpoint_t* aEndpoint)
{
    // todo: if aEndpoint->startIndex == CInvalidRegAdr then allocate the next free endpoint index

    // avoid insertion of endpoint before end
    // and if endpoint list is full
    if( aEndpoint->startIndex < CEpListBaseRegAdr + PT_MAXENDPOINTS + 1 ||
        mEpListEndpoint.length >= PT_MAXENDPOINTS + 1)
    {
        return;
    }

    mEpListEndpoint.length++;

    // insertion is only after mEpListEndpoint possible
    endpoint_t* tmp = &mEpListEndpoint;
    uint32_t epListInd = 1;
    bool insertPend = true;

    while(tmp)
    {
        if(insertPend && (tmp->next == 0 || tmp->next->startIndex > aEndpoint->startIndex))
        {
            insertPend = false;
            aEndpoint->next = tmp->next;
            tmp->next = aEndpoint;
        }

        // update endpoint list
        mEpListPara[epListInd].pPara = &tmp->epId;
        mEpListPara[epListInd].flags = PARA_FLAG_R | PARA_FLAG_P;
        epListInd++;
        tmp = tmp->next;
    }
    
}

void TParaTable::setPara(uint16_t aRegAdr, uint32_t aData)
{
    paraRec_t* tmp = findPara(aRegAdr);

    if(tmp && tmp->flags & PARA_FLAG_W)
    {
        if(tmp->flags & PARA_FLAG_P)
            *tmp->pPara = aData;
        else
            tmp->para = aData;

        if(tmp->flags & PARA_FLAG_FW && tmp->pFAccessCb)
        {
            tmp->pFAccessCb(tmp->cbArg, tmp, true);
        }
    }
}


bool TParaTable::getPara(uint16_t aRegAdr, uint32_t *aData)
{
    paraRec_t* tmp = findPara(aRegAdr);

    if(tmp && tmp->flags & PARA_FLAG_R)
    {
        if(tmp->flags & PARA_FLAG_FR && tmp->pFAccessCb)
        {
            tmp->pFAccessCb(tmp->cbArg, tmp, false);
        }
        if(tmp->flags & PARA_FLAG_P)
            *aData = *tmp->pPara;
        else
            *aData = tmp->para;
        return true;
    }
    else
    {
        return false;
    }
}

bool TParaTable::getParaAdr(uint16_t aRegAdr, uint32_t** aPraRec)
{
    paraRec_t* tmp = findPara(aRegAdr);

    if(tmp && tmp->flags & PARA_FLAG_S)
    {
        *aPraRec = tmp->pPara;
        return true;
    }
    else
    {
        return false;
    }   
}

TParaTable::paraRec_t* TParaTable::findPara(uint16_t index)
{
    endpoint_t* tmp = &mSysEndpoint;
    while(tmp)
    {
        if(tmp->startIndex <= index && (tmp->startIndex + tmp->length) > index)
        {
            return &tmp->para[index - tmp->startIndex];
        }
        tmp = tmp->next;
    }
    return 0;
}

void TParaTable::loadPara()
{
    mStoreLoadEp = &mSysEndpoint;
    mStoreLoadInd = 0;
    calcNVCheckSum(&mNVCheckSum, &mNVLen);
    mStorage->load(loadParaCb, this);
    if(mStoreLoadEp == &mSysEndpoint)
        loadDefault();
};

void TParaTable::storePara()
{
    mStoreLoadEp = &mSysEndpoint;
    mStoreLoadInd = 0;
    calcNVCheckSum(&mNVCheckSum, &mNVLen);
    mStorage->store(mNVLen+1, storeParaCb, this);
};

void TParaTable::calcNVCheckSum(uint32_t* aCheckSum, uint32_t* aNVParaCnt)
{
    uint32_t crc = 0;
    uint32_t len = 0;

    endpoint_t* tmpEp = &mSysEndpoint;
    while(tmpEp)
    {
        paraRec_t* tmpPar = tmpEp->para;
        uint32_t len = tmpEp->length;
        for(uint16_t i = 0; i < len; i++)
        {
            if(tmpPar[i].flags & PARA_FLAG_NV)
            {
                crc = GM_BusDefs::crcCalc(crc, (uint8_t) (tmpEp->startIndex + i));
                crc = GM_BusDefs::crcCalc(crc, (uint8_t) ((tmpEp->startIndex + i) >> 8));
                len++;
            }
        }
        tmpEp = tmpEp->next;
    }
    *aCheckSum = crc;
    *aNVParaCnt = len;
}

bool TParaTable::storeParaCb(void* aArg, uint32_t* aData, uint32_t aLen)
{
    TParaTable* pObj = (TParaTable*) aArg;

    for(uint32_t inc = 0; inc < aLen; inc++)
    {
        if(pObj->mStoreLoadEp == &pObj->mSysEndpoint)
        {
            aData[0] = pObj->mNVCheckSum;
        }
        else
        {
            // find next NV para
            bool found = false;
            endpoint_t* tmpEp = pObj->mStoreLoadEp;
            while(tmpEp && !found)
            {
                paraRec_t* tmpPar = tmpEp->para;
                uint32_t len = tmpEp->length;
                for(uint16_t i = 0; i < len && !found; i++)
                {
                    if(tmpPar[i].flags & PARA_FLAG_NV)
                    {
                        found == true;
                        if(tmpPar[i].flags & PARA_FLAG_P)
                        {
                            aData[inc] = *tmpPar[i].pPara;
                        }
                        else
                        {
                            aData[inc] = tmpPar[i].para;
                        }
                    }
                }
                tmpEp = tmpEp->next;
            }

            pObj->mStoreLoadEp = tmpEp;
            
            if(!found)
            {
                // no further NV data found, store of all NV Data done
                return false;
            }
        }
    }

    return true;
}

bool TParaTable::loadParaCb(void* aArg, uint32_t* aData, uint32_t aLen)
{
    TParaTable* pObj = (TParaTable*) aArg;

    for(uint32_t inc = 0; inc < aLen; inc++)
    {
        if(pObj->mStoreLoadEp == &pObj->mSysEndpoint)
        {
            // check NV Data Checksum
            if(pObj->mNVCheckSum != aData[0])
                return false;
        }
        else
        {
            // find next NV para
            bool found = false;
            endpoint_t* tmpEp = pObj->mStoreLoadEp;
            while(tmpEp && !found)
            {
                paraRec_t* tmpPar = tmpEp->para;
                uint32_t len = tmpEp->length;
                for(uint16_t i = 0; i < len && !found; i++)
                {
                    if(tmpPar[i].flags & PARA_FLAG_NV)
                    {
                        found == true;
                        if(tmpPar[i].flags & PARA_FLAG_P)
                        {
                            *tmpPar[i].pPara = aData[inc];
                        }
                        else
                        {
                            tmpPar[i].para = aData[inc];
                        }

                        if(tmpPar[i].flags & PARA_FLAG_FW)
                        {
                            tmpPar[i].pFAccessCb(tmpPar[i].cbArg, &tmpPar[i], true);
                        }
                    }
                }
                tmpEp = tmpEp->next;
            }

            pObj->mStoreLoadEp = tmpEp;
            
            if(!found)
            {
                // no further NV data found, store of all NV Data done
                return false;
            }
        }
    }

    return true;
}

void TParaTable::paraSaveCb(void* aCbArg, struct paraRec_s* aPParaRec, bool aWrite)
{
    TParaTable* pObj = (TParaTable*) aCbArg;

    switch(aPParaRec->para)
    {
        case 1:
            pObj->storePara();
            break;
        
        case 2:
            pObj->loadPara();
            break;

        defualt:
            break;
    }
}

void TParaTable::paraStartCb(void* aCbArg, struct paraRec_s* aPParaRec, bool aWrite)
{

    TParaTable* pObj = (TParaTable*) aCbArg;

    switch(aPParaRec->para)
    {
        case 1:
            // todo: remove hardware dependend code to
            watchdog_enable(1, 1);
            while(1);
            break;
        
        case 2:
            // todo: remove hardware dependend code to
            reset_usb_boot(0, 0);
            break;

        defualt:
            break;
    }
}

void TParaTable::loadDefault(uint16_t aRegAdr)
{
    // todo: restore default values
}
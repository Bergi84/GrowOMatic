#include "paraTable.h"
#include "gm_epLib.h"
#include "pico/bootrom.h"
#include "hardware/watchdog.h"
#include <string.h>

TParaTable::TParaTable() : 
// init system parameter list
mSysPara( (paraRec_t[cParaListLength]) {
    [PARA_UID] =          {.para = 0,                                       .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_UID]},
    [PARA_TYPE] =         {.para = 0,                                       .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_TYPE]},
    [PARA_FWVERSION] =    {.para = VER_COMBO,                               .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_FWVERSION]},
    [PARA_SAVE] =         {.para = 0,                                       .pFAccessCb = paraSaveCb,   .cbArg = this,    .defs = &cParaList[PARA_SAVE]},
    [PARA_START] =        {.para = 0,                                       .pFAccessCb = paraStartCb,  .cbArg = this,    .defs = &cParaList[PARA_START]},
    [PARA_DEVNAME0] =     {.pPara = (uint32_t*) &mDevName[0],               .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_DEVNAME0]},
    [PARA_DEVNAME1] =     {.pPara = (uint32_t*) &mDevName[4],               .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_DEVNAME1]},
    [PARA_DEVNAME2] =     {.pPara = (uint32_t*) &mDevName[8],               .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_DEVNAME2]},
    [PARA_DEVNAME3] =     {.pPara = (uint32_t*) &mDevName[12],              .pFAccessCb = 0,            .cbArg = 0,       .defs = &cParaList[PARA_DEVNAME3]}
    }),

// init Endpoint List Endpoint 
mEpListEndpoint( (endpoint_t) {
    { { 
        .baseInd = CEpListBaseRegAdr,
        .type = (uint16_t)EPT_EPLIST    
    } }, 
    .length = 1, 
    .para = mEpListPara,
    .next = 0,
    .typeName = "epList",
}),
// init system endpoint
mSysEndpoint( (endpoint_t) { 
    { { 
        .baseInd = CSystemBaseRegAdr,
        .type = (uint16_t)EPT_SYSTEM    
    } }, 
    .length = cParaListLength, 
    .para = mSysPara,
    .next = &mEpListEndpoint,
    .typeName = cTypeName
})
{
    strcpy(mSysEndpoint.epName, mSysEndpoint.typeName);
    strcpy(mEpListEndpoint.epName, mEpListEndpoint.typeName);

    for(int i = 0; i < PT_MAXENDPOINTS; i++)
    {
        mEpListEndpointDefs[i].flags = PARA_FLAG_R | PARA_FLAG_P;
        mEpListEndpointDefs[i].paraName = 0;
        mEpListPara[i] = {.para = EPT_INVALID, .pFAccessCb = 0, .cbArg = 0, .defs = &mEpListEndpointDefs[i]};
    }
}

void TParaTable::init(uint32_t aUniqueId, devType_t aDevType, TStorage* aStorage)
{
    while(CInvalidUid == aUniqueId);

    mSysPara[PARA_UID].para = aUniqueId;
    mSysPara[PARA_TYPE].para = (uint32_t) aDevType;
    mStorage = aStorage;
}

void TParaTable::addEndpoint(endpoint_t* aEndpoint)
{
    // todo: if aEndpoint->epId.baseInd == CInvalidReg then allocate the next free endpoint index
    while(aEndpoint->epId.baseInd == CInvalidReg);

    // avoid insertion of endpoint before end
    // and if endpoint list is full
    if( aEndpoint->epId.baseInd < CEpListBaseRegAdr + PT_MAXENDPOINTS + 1 ||
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
        if(insertPend && (tmp->next == 0 || tmp->next->epId.baseInd > aEndpoint->epId.baseInd))
        {
            insertPend = false;
            aEndpoint->next = tmp->next;
            tmp->next = aEndpoint;
        }

        // update endpoint list
        mEpListPara[epListInd].pPara = &tmp->epId.id;
        epListInd++;
        tmp = tmp->next;
    }
    
}

errCode_T TParaTable::setPara(uint16_t aRegAdr, uint32_t aData)
{
    paraRec_t* tmp = findPara(aRegAdr);

    if(tmp && tmp->defs->flags & PARA_FLAG_W)
    {
        if(tmp->defs->flags & PARA_FLAG_P)
            *tmp->pPara = aData;
        else
            tmp->para = aData;

        if(tmp->defs->flags & PARA_FLAG_FW && tmp->pFAccessCb)
        {
            tmp->pFAccessCb(tmp->cbArg, tmp, true);
        }
        return EC_SUCCESS;
    }
    else
    {
        if(tmp)
            return EC_PROTECTED;
        else
            return EC_INVALID_REGADR;
    }
}


errCode_T TParaTable::getPara(uint16_t aRegAdr, uint32_t *aData)
{
    paraRec_t* tmp = findPara(aRegAdr);

    if(tmp && tmp->defs->flags & PARA_FLAG_R)
    {
        if(tmp->defs->flags & PARA_FLAG_FR && tmp->pFAccessCb)
        {
            tmp->pFAccessCb(tmp->cbArg, tmp, false);
        }
        if(tmp->defs->flags & PARA_FLAG_P)
            *aData = *tmp->pPara;
        else
            *aData = tmp->para;
        return EC_SUCCESS;
    }
    else
    {
        if(tmp)
            return EC_PROTECTED;
        else
            return EC_INVALID_REGADR;
    }
}

bool TParaTable::getParaAdr(uint16_t aRegAdr, uint32_t** aPraRec)
{
    paraRec_t* tmp = findPara(aRegAdr);

    if(tmp && tmp->defs->flags & PARA_FLAG_S)
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
        if(tmp->epId.baseInd <= index && (tmp->epId.baseInd + tmp->length) > index)
        {
            return &tmp->para[index - tmp->epId.baseInd];
        }
        tmp = tmp->next;
    }
    return 0;
}

TParaTable::endpoint_t* TParaTable::findEp(uint16_t baseInd)
{
    endpoint_t* tmp = &mSysEndpoint;
    while(tmp)
    {
        if(tmp->epId.baseInd == baseInd)
        {
            return tmp;
        }
        tmp = tmp->next;
    }
    return 0;
}

void TParaTable::loadPara()
{
    mStoreLoadEp = &mSysEndpoint;
    mStoreLoadCnt = 0;
    mStoreLoadInd = 0;
    mStoreLoadDone = false;
    calcNVCheckSum(&mNVCheckSum, &mNVLen);
    mStorage->load(loadParaCb, this);
    if(mStoreLoadEp == &mSysEndpoint)
        loadDefault();
};

void TParaTable::storePara()
{
    mStoreLoadEp = &mSysEndpoint;
    mStoreLoadCnt = 0;
    mStoreLoadInd = 0;
    mStoreLoadDone = false;
    calcNVCheckSum(&mNVCheckSum, &mNVLen);
    mStorage->store(mNVLen+1, storeParaCb, this);
};

void TParaTable::clearPara()
{
    mStorage->store(mNVLen+1, storeParaCb, this);
}

void TParaTable::calcNVCheckSum(uint32_t* aCheckSum, uint32_t* aNVParaCnt)
{
    uint32_t crc = 0;
    uint32_t cnt = 0;

    endpoint_t* tmpEp = &mSysEndpoint;
    while(tmpEp)
    {
        paraRec_t* tmpPar = tmpEp->para;
        uint32_t len = tmpEp->length;
        for(uint16_t i = 0; i < len; i++)
        {
            if(tmpPar[i].defs->flags & PARA_FLAG_NV)
            {
                crc = GM_BusDefs::crcCalc(crc, (uint8_t) (tmpEp->epId.baseInd + i));
                crc = GM_BusDefs::crcCalc(crc, (uint8_t) ((tmpEp->epId.baseInd + i) >> 8));
                cnt++;
            }
        }
        tmpEp = tmpEp->next;
    }
    *aCheckSum = crc;
    *aNVParaCnt = cnt;
}



bool TParaTable::storeParaCb(void* aArg, uint32_t* aData, uint32_t aLen)
{
    TParaTable* pObj = (TParaTable*) aArg;

    endpoint_t* tmpEp = pObj->mStoreLoadEp;
    uint16_t ind = pObj->mStoreLoadInd;
    paraRec_t* tmpPar = tmpEp->para;
    uint32_t len = tmpEp->length;

    for(uint32_t inc = 0; inc < aLen; inc++)
    {
        if(pObj->mStoreLoadCnt == 0)
        {
            aData[0] = pObj->mNVCheckSum;
        }
        else
        {
            // find next NV para
            bool found = false;

            while(tmpEp && !found)
            {
                if(tmpPar[ind].defs->flags & PARA_FLAG_NV)
                {
                    found = true;
                    if(tmpPar[ind].defs->flags & PARA_FLAG_P)
                    {
                        aData[inc] = *tmpPar[ind].pPara;
                    }
                    else
                    {
                        aData[inc] = tmpPar[ind].para;
                    }
                }

                if(ind < len)
                {
                    ind++;
                }
                else
                {
                    tmpEp = tmpEp->next;
                    ind = 0;

                    if(tmpEp != 0)
                    {
                        len = tmpEp->length;
                        tmpPar = tmpEp->para;
                    }
                }
            }
            
            if(tmpEp == 0)
            {
                // no further NV data found, store of all NV Data done
                return false;
            }
        }
        pObj->mStoreLoadCnt++;
    }

    pObj->mStoreLoadEp = tmpEp;
    pObj->mStoreLoadInd = ind;

    if(pObj->mStoreLoadCnt == pObj->mNVLen + 1)
        pObj->mStoreLoadDone = true;

    return true;
}

bool TParaTable::loadParaCb(void* aArg, uint32_t* aData, uint32_t aLen)
{
    TParaTable* pObj = (TParaTable*) aArg;

    endpoint_t* tmpEp = pObj->mStoreLoadEp;
    uint16_t ind = pObj->mStoreLoadInd;
    paraRec_t* tmpPar = tmpEp->para;
    uint32_t len = tmpEp->length;

    for(uint32_t inc = 0; inc < aLen; inc++)
    {
        if(pObj->mStoreLoadCnt == 0)
        {
            aData[0] = pObj->mNVCheckSum;
        }
        else
        {
            // find next NV para
            bool found = false;

            while(tmpEp && !found)
            {
                if(tmpPar[ind].defs->flags & PARA_FLAG_NV)
                {
                    found = true;
                    if(tmpPar[ind].defs->flags & PARA_FLAG_P)
                    {
                        *tmpPar[ind].pPara = aData[inc];
                    }
                    else
                    {
                        tmpPar[ind].para = aData[inc];
                    }

                    if(tmpPar[ind].defs->flags & PARA_FLAG_FW)
                    {
                        tmpPar[ind].pFAccessCb(tmpPar[ind].cbArg, &tmpPar[ind], true);
                    }
                }
                   
                if(ind < len)
                {
                    ind++;
                }
                else
                {
                    tmpEp = tmpEp->next;
                    ind = 0;

                    if(tmpEp != 0)
                    {
                        len = tmpEp->length;
                        tmpPar = tmpEp->para;
                    }
                }
            }
            
            if(tmpEp == 0)
            {
                // no further NV data found, store of all NV Data done
                return false;
            }
        }
        pObj->mStoreLoadCnt++;
    }

    pObj->mStoreLoadEp = tmpEp;
    pObj->mStoreLoadInd = ind;

    if(pObj->mStoreLoadCnt == pObj->mNVLen + 1)
        pObj->mStoreLoadDone = true;

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
            pObj->clearPara();
            break;
    }
}

void TParaTable::paraStartCb(void* aCbArg, struct paraRec_s* aPParaRec, bool aWrite)
{

    TParaTable* pObj = (TParaTable*) aCbArg;

    switch(aPParaRec->para)
    {
        case 1:
            // todo: remove hardware dependend code
            watchdog_enable(1, 1);
            while(1);
            break;
        
        case 2:
            // todo: remove hardware dependend code
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
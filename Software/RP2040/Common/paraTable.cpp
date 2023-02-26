#include "paraTable.h"
#include "gm_epLib.h"
#include <string.h>

TParaTable::TParaTable() :

// init Endpoint List Endpoint 
mEpListEndpoint( (endpoint_t) {
    { { 
        .baseInd = CEpListBaseRegAdr,
        .type = (uint16_t)EPT_EPLIST    
    } }, 
    .length = 0, 
    .para = mEpListPara,
    .next = 0,
    .typeName = "epList",
})
{

}

void TParaTable::init(TStorage* aStorage)
{
    mStorage = aStorage;

    strcpy(mEpListEndpoint.epName, mEpListEndpoint.typeName);

    for(int i = 0; i < PT_MAXENDPOINTS; i++)
    {
        mEpListPara[i] = {.para = EPT_INVALID, .pFAccessCb = 0, .cbArg = 0, .defs = &mEpListEndpointDefs};
    }

    mTmpPara.cbArg = 0;
    mTmpPara.pFAccessCb = 0;

    mActiveCb = 0;
    mActiveCbArg = 0;
}

void TParaTable::addEndpoint(endpoint_t* aEndpoint)
{
    if(aEndpoint->epId.type == EPT_SYSTEM)
    {
        aEndpoint->epId.baseInd = CSystemBaseRegAdr;
        mRootEp = aEndpoint;
        aEndpoint->next = &mEpListEndpoint;
        return;
    }


    while(aEndpoint->epId.baseInd == CInvalidReg);

    // avoid insertion of endpoint before endpointlist
    // and if endpoint list is full
    if( aEndpoint->epId.baseInd < CEpListBaseRegAdr + PT_MAXENDPOINTS + 1 ||
        mEpListEndpoint.length >= PT_MAXENDPOINTS + 1)
    {
        return;
    }

    mEpListEndpoint.length++;

    // insertion is only after mEpListEndpoint possible
    endpoint_t* tmp = &mEpListEndpoint;
    uint32_t epListInd = 0;
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
        if(epListInd > 0)
            mEpListPara[epListInd - 1].para = tmp->epId.id;
        epListInd++;
        tmp = tmp->next;
    }
    
}

errCode_T TParaTable::setPara(uint16_t aRegAdr, uint32_t aData)
{
    paraRec_t* tmp = findPara(aRegAdr);

    if(mActiveCb != 0)
        mActiveCb(mActiveCbArg);

    if(tmp && tmp->defs->flags & PARA_FLAG_W)
    {
        if(tmp->defs->flags & PARA_FLAG_P)
            *tmp->pPara = aData;
        else
            tmp->para = aData;

        if((tmp->defs->flags & PARA_FLAG_FW) != 0 && tmp->pFAccessCb)
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

    if(mActiveCb != 0)
        mActiveCb(mActiveCbArg);

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

errCode_T TParaTable::getParaName(uint16_t aRegAdr, const char **aData)
{
    paraRec_t* tmp = findPara(aRegAdr);

    if(tmp)
    {
        *aData = tmp->defs->paraName;
        return EC_SUCCESS;
    }
    else
    {
        *aData = 0;
        return EC_INVALID_REGADR;
    }
}

errCode_T TParaTable::getParaPer(uint16_t aRegAdr, uint32_t *aData)
{
    paraRec_t* tmp = findPara(aRegAdr);
    
    if(tmp)
    {
        *aData = tmp->defs->flags;
        return EC_SUCCESS;
    }
    else
    {
        *aData = 0;
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

void TParaTable::setActiveCb(void (*aCb)(void* aArg), void* aArg)
{
    mActiveCb = aCb;
    mActiveCbArg = aArg;
}

TParaTable::paraRec_t* TParaTable::findPara(uint16_t index)
{
    endpoint_t* tmp = mRootEp;
    while(tmp)
    {
        if(tmp->epId.baseInd <= index)
        {
            if( (tmp->epId.baseInd + tmp->length) > index)
            {
                return &tmp->para[index - tmp->epId.baseInd];
            } else if ( (tmp->epId.baseInd + tmp->length + 4) > index)
            {
                mTmpPara.pPara = (uint32_t*) &tmp->epName[(index - tmp->epId.baseInd - tmp->length) * 4], 
                mTmpPara.defs = &CEpNameDefs[index - tmp->epId.baseInd - tmp->length];
                return &mTmpPara;
            }
        }
        tmp = tmp->next;
    }
    return 0;
}

TParaTable::endpoint_t* TParaTable::findEp(uint16_t baseInd)
{
    endpoint_t* tmp = mRootEp;
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
    mStoreLoadEp = mRootEp;
    mStoreLoadCnt = 0;
    mStoreLoadInd = 0;
    mStoreLoadDone = false;
    calcNVCheckSum(&mNVCheckSum, &mNVLen);
    mStorage->load(loadParaCb, this);

    /*
    if(mStoreLoadInd == 0)
        loadDefault();
    */
};

void TParaTable::storePara()
{
    mStoreLoadEp = mRootEp;
    mStoreLoadCnt = 0;
    mStoreLoadInd = 0;
    mStoreLoadDone = false;
    calcNVCheckSum(&mNVCheckSum, &mNVLen);
    mStorage->store(mNVLen+1, storeParaCb, this);
};

void TParaTable::clearPara()
{
    mStorage->clear();
}

void TParaTable::calcNVCheckSum(uint32_t* aCheckSum, uint32_t* aNVParaCnt)
{
    uint32_t crc = 0;
    uint32_t cnt = 0;

    endpoint_t* tmpEp = mRootEp;
    while(tmpEp)
    {
        // epName
        uint32_t pos = 0;
        while(tmpEp->epName[pos] != 0)
        {
            crc = GM_BusDefs::crcCalc(crc, (uint8_t) tmpEp->epName[pos]);
            pos++;
        }

        // non volatile parameter namens
        uint32_t len = tmpEp->length;
        for(uint16_t i = 0; i < len; i++)
        {
            paraRec_t* tmpPar = &tmpEp->para[i];
            if(tmpPar->defs->flags & PARA_FLAG_NV)
            {
                uint32_t pos = 0;
                while(tmpPar->defs->paraName[pos] != 0)
                {
                    crc = GM_BusDefs::crcCalc(crc, (uint8_t) tmpPar->defs->paraName[pos]);
                    pos++;
                }
                cnt++;
            }
        }

        // epname are virtual parameters and always non volatile
        if(tmpEp->epId.type != EPT_SYSTEM && tmpEp->epId.type != EPT_EPLIST)
        {
            for(uint16_t i = len; i < len+4; i++)
            {
                uint32_t pos = 0;
                while(CEpNameDefs[i - len].paraName[pos] != 0)
                {
                    crc = GM_BusDefs::crcCalc(crc, (uint8_t) CEpNameDefs[i - len].paraName[pos]);
                    pos++;
                }
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
                if(ind < len)
                {
                    // store real parameters
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
                }
                else
                {
                    // store endpoint name as virtual parameter
                    if(tmpEp->epId.type != EPT_SYSTEM && tmpEp->epId.type != EPT_EPLIST)
                    {
                        found = true;
                        aData[inc] = *((uint32_t*) &tmpEp->epName[(ind - len)*4]);
                    }
                }

                if(ind < len + 3)
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
            if(aData[0] != pObj->mNVCheckSum)
                return false;
            
        }
        else
        {
            // find next NV para
            bool found = false;

            while(tmpEp && !found)
            {
                if(ind < len)
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
                }
                else
                {
                    // load endpoint name as virtual parameter
                    if(tmpEp->epId.type != EPT_SYSTEM && tmpEp->epId.type != EPT_EPLIST)
                    {
                        found = true;
                        *((uint32_t*) &tmpEp->epName[(ind - len)*4]) = aData[inc];
                    }
                } 

                if(ind < len + 3)
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

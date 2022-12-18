#include "paraTable.h"
#include "gm_epLib.h"

TParaTable::TParaTable() : 
mSysPara( (paraRec_t[mSysParaLen]) {
    /*   0 uniqueId         */ {.para = 0,          .pFAccessCb = 0, .cbArg = 0, .flags = PARA_FLAG_R},
    /*   1 deviceType       */ {.para = 0,          .pFAccessCb = 0, .cbArg = 0, .flags = PARA_FLAG_R},
    /*   2 fwVersion        */ {.para = VER_COMBO,  .pFAccessCb = 0, .cbArg = 0, .flags = PARA_FLAG_R},
    /*   3 busMaster        */ {.para = 0,          .pFAccessCb = 0, .cbArg = 0, .flags = PARA_FLAG_RW | PARA_FLAG_NV | PARA_FLAG_FW},
    /*   4 savePara         */ {.para = 0,          .pFAccessCb = 0, .cbArg = 0, .flags = PARA_FLAG_W | PARA_FLAG_FW},
    /*   5 start            */ {.para = 0,          .pFAccessCb = 0, .cbArg = 0, .flags = PARA_FLAG_W | PARA_FLAG_FW},
    }),

mEpListEndpoint( (endpoint_t) {
    { { 
        .startIndex = CEpListBaseRegAdr,
        .type = (uint16_t)EPT_EPLIST    
    } }, 
    .length = 1, 
    .para = mEpListPara,
    .next = 0
}),
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

void TParaTable::init(uint32_t aUniqueId, devType_t aDevType)
{
    mSysPara[0].para = aUniqueId;
    mSysPara[1].para = (uint32_t) aDevType;
}

void TParaTable::addEndpoint(endpoint_t* aEndpoint)
{
    // avoid insertion of endpoint before end
    // and if endpointlist is full
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
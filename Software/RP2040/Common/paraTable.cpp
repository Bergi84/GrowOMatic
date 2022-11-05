#include "paraTable.h"

void TParaTable::init()
{
    mTableRoot = 0;
}

void TParaTable::addSubTable(subTable_t* aSubTable)
{
    if(mTableRoot == 0 || mTableRoot->startIndex > aSubTable->startIndex)
    {
        aSubTable->next = mTableRoot;
        mTableRoot = aSubTable;
    }
    else
    {
        subTable_t* tmp = mTableRoot;
        while(tmp)
        {
            if(tmp->next == 0 || tmp->next->startIndex > aSubTable->startIndex)
            {
                aSubTable->next = tmp->next;
                tmp->next = aSubTable;
                break;
            }
            tmp = tmp->next;
        }
    }
}

void TParaTable::setPara(uint16_t aRegAdr, uint32_t aData)
{
    paraRec_t* tmp = findPara(aRegAdr);

    if(tmp && tmp->flags & PARA_FLAG_W)
    {
        *tmp->pPara = aData;
        if(tmp->pFAccessCb)
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
        *aData = *tmp->pPara;
        if(tmp->pFAccessCb)
        {
            tmp->pFAccessCb(tmp->cbArg, tmp, true);
        }
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
    subTable_t* tmp = mTableRoot;
    while(tmp)
    {
        if(tmp->startIndex <= index && (tmp->startIndex + tmp->length) > index)
        {
            return &tmp->para[index - tmp->startIndex];
        }
    }
    return 0;
}
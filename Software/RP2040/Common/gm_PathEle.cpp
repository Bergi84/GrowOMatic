#include "gm_PathEle.h"

TPathEle::TPathEle()
{
    mType = POT_NONE;
}

void TPathEle::init(TParaTable::endpoint_t* aEp, uint16_t aOffInd, TParaTable* aPt)
{
    mType = POT_LOCREG;
    mEp.loc = aEp;
    mOffInd = aOffInd;
    mPt = aPt;
}

void TPathEle::init(TEpBase* aEp, uint16_t aOffInd)
{
    mType = POT_REMREG;
    mEp.rem = aEp;
    mOffInd = aOffInd;
}

void TPathEle::deInit()
{
    mType = POT_NONE;
}

bool TPathEle::isFolder()
{
    return mType == POT_FOLDER;
}

bool TPathEle::isFile()
{
    return mType == POT_REMREG || mType == POT_LOCREG;
}

errCode_T TPathEle::getValue(void (*aCb) (void* aArg, uint32_t* aVal, errCode_T aStatus), void* aArg)
{
    if(mType == POT_REMREG)
    {
        // remote request
        return mEp.rem->reqPara(mOffInd, aCb, aArg);
    }
    else
    {
        // local request
        uint32_t val;
        uint16_t regAdr = mEp.loc->epId.baseInd + mOffInd;
        errCode_T ec = mPt->getPara(regAdr, &val);

        if(ec != EC_SUCCESS)
            return ec;

        aCb(aArg, &val, ec);
    }
    return EC_SUCCESS;
}

errCode_T TPathEle::setValue(uint32_t aVal, void (*aCb) (void* aArg, uint32_t* aVal, errCode_T aStatus), void* aArg)
{
    if(mType == POT_REMREG)
    {
        // remote request
        return mEp.rem->setPara(mOffInd, aVal, aCb, aArg);
    }
    else
    {
        // local request
        uint16_t regAdr = mEp.loc->epId.baseInd + mOffInd;
        errCode_T ec = mPt->setPara(regAdr, aVal);

        if(ec != EC_SUCCESS)
            return ec;

        aCb(aArg, &aVal, ec);
    }
    return EC_SUCCESS;
}

uint32_t TPathEle::getPer()
{
    bool readPer;
    if(mType == POT_REMREG)
    {
        // remote request
        return mEp.rem->getParaPer(mOffInd);
    }
    else
    {
        // local request
        return mEp.loc->para[mOffInd].defs->flags;
    }
}

const char* TPathEle::getName()
{
    bool readPer;
    if(mType == POT_REMREG)
    {
        // remote request
        return mEp.rem->getParaName(mOffInd);
    }
    else
    {
        // local request
        return mEp.loc->para[mOffInd].defs->paraName;
    }
}

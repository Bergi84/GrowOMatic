#include "gm_PathEle.h"

TPathEle::TPathEle()
{
    mType = POT_NONE;
}

void TPathEle::init(TParaTable::endpoint_t* aEp, TParaTable* aPt, uint16_t aOffInd)
{
    if(aOffInd == CInvalidReg)
        mType = POT_LOCEP;
    else
        mType = POT_LOCREG;

    mEp.loc = aEp;
    mOffInd = aOffInd;
    mPt = aPt;
}

void TPathEle::init(TEpBase* aEp, uint16_t aOffInd)
{
    if(aOffInd == CInvalidReg)
        mType = POT_REMEP;
    else
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
    return mType == POT_FOLDER || mType == POT_LOCEP || mType == POT_REMEP;
}

bool TPathEle::isFile()
{
    return mType == POT_REMREG || mType == POT_LOCREG;
}

bool TPathEle::isEndPoint()
{
    return mType == POT_LOCEP || mType == POT_REMEP;
}

errCode_T TPathEle::getValue(void (*aCb) (void* aArg, uint32_t* aVal, errCode_T aStatus), void* aArg)
{
    if(isFile())
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
            return EC_SUCCESS;
        }
    }
    return EC_INVALID_PATH;
}

errCode_T TPathEle::setValue(uint32_t aVal, void (*aCb) (void* aArg, uint32_t* aVal, errCode_T aStatus), void* aArg)
{
    if(isFile())
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
            return EC_SUCCESS;
        }
    }
    return EC_INVALID_PATH;
}

uint32_t TPathEle::getPer()
{
    if(isFile())
    {
        if(mType == POT_REMREG)
        {
            // remote request
            return mEp.rem->getParaPer(mOffInd);
        }
        else
        {
            // local request
            uint32_t retVal;
            uint16_t regAdr = mEp.loc->epId.baseInd + mOffInd;
            mPt->getParaPer( regAdr, &retVal);
            return retVal;
        }
    }
    return 0;
}

const char* TPathEle::getName()
{
    if(isFile())
    {
        if(mType == POT_REMREG)
        {
            // remote request
            return mEp.rem->getParaName(mOffInd);
        }
        else
        {
            // local request
            const char* retVal;
            uint16_t regAdr = mEp.loc->epId.baseInd + mOffInd;
            mPt->getParaName( regAdr, &retVal);
            return retVal;
        }
    }
    
    if(isEndPoint())
    {
        if(mType == POT_REMEP)
        {
            // remote request
            return mEp.rem->getEpName();
        }
        else
        {
            // local request
            return mEp.loc->epName;
        }
    }
    return 0;
}

epType_t TPathEle::getEpType()
{
    if(isEndPoint())
    {
        if(mType == POT_REMEP)
        {
            // remote request
            return mEp.rem->getType();
        }
        else
        {
            // local request
            return (epType_t) mEp.loc->epId.type;
        }        
    }
    return EPT_INVALID;
}

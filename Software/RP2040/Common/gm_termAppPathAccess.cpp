#include "gm_termAppPathAccess.h"
#include "gm_termPathMng.h"

#include <stdlib.h>

gm_termAppPathAccess::gm_termAppPathAccess()
{

}

gm_termAppPathAccess::~gm_termAppPathAccess()
{

}

void gm_termAppPathAccess::start(uint8_t* aStartArg)
{
    uint32_t pos = findNextS((char*) aStartArg);

    GM_termPathMng* pathMng = (GM_termPathMng*) getPathMng();

    pathObj_t obj = pathMng->getPathObj((char*) aStartArg, pos);
    
    if(obj.type != POT_REMREG || obj.type != POT_LOCREG)
    {
        printErr(EC_INVALID_PATH);
        done();
        return;
    }

    uint32_t valPos = findNextC((char*) &aStartArg[pos]);

    if(valPos = 0)
    {
        // no further argument, read access
        readObj(&obj);
    }
    else
    {
        // write request, parse value
        char* valStr = (char*) &aStartArg[pos + valPos];
        uint32_t valLen = findNextS(valStr);
        
        char* valEnd = valStr;
        uint32_t val = strtol(valStr, &valEnd, 0);

        if(valEnd == valStr)
        {
                printErr(EC_INVALID_VALUE);
                done();
                return;         
        }

        writeObj(&obj, val);
    }
}

void gm_termAppPathAccess::readObj(pathObj_t* aObj)
{
    mLastObj = aObj;

    GM_termPathMng* pathMng = (GM_termPathMng*) getPathMng();

    if(aObj->type == POT_REMREG)
    {
        // remote request
        TEpBase* ep = (TEpBase*) aObj->objP;
        errCode_T ec = ep->reqPara(aObj->offInd, readCb, this);
        if(ec != EC_SUCCESS)
        {
            printErr(ec);
            done();
            return;
        }
    }
    else
    {
        // local request
        TParaTable::endpoint_t* ep = (TParaTable::endpoint_t*) aObj->objP;
        uint32_t val;
        uint16_t regAdr = ep->epId.baseInd + aObj->offInd;
        errCode_T ec = pathMng->getParatable()->getPara(regAdr, &val);
        readCb(this, &val, ec);
    }
}

void gm_termAppPathAccess::writeObj(pathObj_t* aObj, uint32_t aVal)
{
    mLastObj = aObj;

    GM_termPathMng* pathMng = (GM_termPathMng*) getPathMng();

    if(aObj->type == POT_REMREG)
    {
        // remote request
        TEpBase* ep = (TEpBase*) aObj->objP;
        errCode_T ec = ep->setPara(aObj->offInd, aVal, writeCb, this);

        if(ec != EC_SUCCESS)
        {
            printErr(ec);
            done();
            return;
        }
    }
    else
    {
        // local request
        TParaTable::endpoint_t* ep = (TParaTable::endpoint_t*) aObj->objP;
        uint16_t regAdr = ep->epId.baseInd + aObj->offInd;
        errCode_T ec = pathMng->getParatable()->setPara(regAdr, aVal);
        writeCb(this, &aVal, ec);
    }
}

void gm_termAppPathAccess::parse(uint8_t aChar)
{
    // we ignore all inputs
}

void gm_termAppPathAccess::writeCb (void* aArg, uint32_t* aVal, errCode_T aStatus)
{
    gm_termAppPathAccess* pObj = (gm_termAppPathAccess* )aArg;

    if(aStatus != EC_SUCCESS)
    {
        pObj->printErr(aStatus);
        pObj->done();
    }
    else
    {
        pObj->readObj(pObj->mLastObj);
    }
}

void gm_termAppPathAccess::readCb (void* aArg, uint32_t* aVal, errCode_T aStatus)
{
    gm_termAppPathAccess* pObj = (gm_termAppPathAccess* )aArg;

    if(aStatus != EC_SUCCESS)
    {
        pObj->printErr(aStatus);
    }
    else
    {
        // todo: print value
    }

    pObj->done();
}
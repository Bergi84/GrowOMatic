#include "gm_termAppPathAccess.h"
#include "gm_termPathMng.h"
#include <stdio.h>
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
    
    if(obj.type != POT_REMREG && obj.type != POT_LOCREG)
    {
        printErr(EC_INVALID_PATH);
        done();
        return;
    }

    uint32_t valPos = findNextC((char*) &aStartArg[pos]);

    if(valPos == 0)
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
        bool readPer;
        const char* varName;
        if(pObj->mLastObj->type == POT_REMREG)
        {
            // remote request
            TEpBase* ep = (TEpBase*) pObj->mLastObj->objP;
            readPer = (ep->getParaPer(pObj->mLastObj->offInd) & PARA_FLAG_R) != 0;
            varName = ep->getParaName(pObj->mLastObj->offInd);
        }
        else
        {
            // local request
            TParaTable::endpoint_t* ep = (TParaTable::endpoint_t*) pObj->mLastObj->objP;
            readPer = (ep->para[pObj->mLastObj->offInd].defs->flags & PARA_FLAG_R) != 0;
            varName = ep->para[pObj->mLastObj->offInd].defs->paraName;
        }

        if(readPer)
        {
            // if reading is allowed readback value
            pObj->readObj(pObj->mLastObj);
        }
        else
        {
            // print written value if there is no read permission
            pObj->putString(varName, strlen(varName));
            pObj->putString(" = ", 3);

            char printBuf[16];
            snprintf(printBuf, 16, "%u", *aVal);
            pObj->putString(printBuf, 16);
            pObj->putChar('\r');
            pObj->putChar('\n');
        }
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
        const char* varName;

        if(pObj->mLastObj->type == POT_REMREG)
        {
            // remote request
            TEpBase* ep = (TEpBase*) pObj->mLastObj->objP;
            varName = ep->getParaName(pObj->mLastObj->offInd);
        }
        else
        {
            // local request
            TParaTable::endpoint_t* ep = (TParaTable::endpoint_t*) pObj->mLastObj->objP;
            varName = ep->para[pObj->mLastObj->offInd].defs->paraName;
        }

        // print readed value
        pObj->putString(varName, strlen(varName));
        pObj->putString(" = ", 3);

        char printBuf[16];
        snprintf(printBuf, 16, "%u", *aVal);
        pObj->putString(printBuf, 16);
        pObj->putChar('\r');
        pObj->putChar('\n');
    }

    pObj->done();
}
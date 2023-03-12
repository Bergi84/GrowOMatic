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

    pathMng->getPathObj((char*) aStartArg, pos, &mPathEle);
    
    if(!mPathEle.isFile())
    {
        printErr(EC_NOTAFILE);
        done();
        return;
    }

    uint32_t valPos = findNextC((char*) &aStartArg[pos]);

    if(valPos == 0)
    {
        // no further argument, read access
        errCode_T ec = mPathEle.getValue(readCb, this);

        if(ec != EC_SUCCESS)
        {
            printErr(ec);
            done();
        }
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

        errCode_T ec = mPathEle.setValue(val, writeCb, this);

        if(ec != EC_SUCCESS)
        {
            printErr(ec);
            done();
        }
    }
}

void gm_termAppPathAccess::parse(uint8_t aChar)
{
    // we ignore all inputs
}

void gm_termAppPathAccess::exit()
{
    
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
        if((pObj->mPathEle.getPer() & PARA_FLAG_R) != 0)
        {
            // if reading is allowed readback value
            errCode_T ec = pObj->mPathEle.getValue(readCb, pObj);
            if(ec != EC_SUCCESS)
            {
                pObj->printErr(ec);
                pObj->done();
            }
        }
        else
        {
            const char* varName = pObj->mPathEle.getName();

            // print written value if there is no read permission
            pObj->putString(varName, strlen(varName));
            pObj->putString(" = ", 3);

            char printBuf[16];
            snprintf(printBuf, 16, "%u", *aVal);
            pObj->putString(printBuf, 16);
            pObj->putChar('\r');
            pObj->putChar('\n');

            pObj->done();
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
        const char* varName = pObj->mPathEle.getName();

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
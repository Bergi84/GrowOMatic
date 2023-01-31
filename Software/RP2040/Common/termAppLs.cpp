#include "termAppLs.h"

TTermAppLs::TTermAppLs()
{
    mKeyPhrase = mName;
}

TTermAppLs::~TTermAppLs()
{

}

void TTermAppLs::start(uint8_t* aStartArg)
{
    uint32_t pos = findNextS((char*) aStartArg);

    char* pathStr;
    if(pos != 0)
    {
        pathStr = (char*)aStartArg;
    }
    else
    {
        pathStr = 0;
    }

    char strBuf[TPM_FOLDER_MAX_LEN + 1];
    uint32_t subPathInd = 0;

    uint32_t noBytes = getPathMng()->getSubPath(subPathInd, strBuf, TPM_FOLDER_MAX_LEN + 1, pathStr);

    while(noBytes != 0)
    {
        for(int i = noBytes; i < TPM_FOLDER_MAX_LEN; i++)
            strBuf[i] = ' ';
        
        strBuf[TPM_FOLDER_MAX_LEN] = 0;

        putString(strBuf, TPM_FOLDER_MAX_LEN+1);

        if((subPathInd+1)%4 == 0)
        {
            putChar('\r');
            putChar('\n');
        }

        subPathInd++;
        noBytes = getPathMng()->getSubPath(subPathInd, strBuf, TPM_FOLDER_MAX_LEN + 1, pathStr);
    }

    if((subPathInd)%4 != 0)
    {
        putChar('\r');
        putChar('\n');
    }

    done();
}

void TTermAppLs::parse(uint8_t aChar)
{

}
#include "termAppCd.h"

TTermAppCd::TTermAppCd()
{
    mKeyPhrase = mName;
}

TTermAppCd::~TTermAppCd()
{

}

void TTermAppCd::start(uint8_t* aStartArg)
{
    uint32_t pos;
    while(  aStartArg[pos] != ' ' &&
            aStartArg[pos] != '\n' &&
            aStartArg[pos] != '\r' && 
            aStartArg[pos] != 0)
        pos++;

    if(getPathMng()->setAktPath((char*)aStartArg, pos) != EC_SUCCESS)
    {
        const char* str = "invalid path\r\n";
        putString(str, strlen(str));
    }
    done();
}

void TTermAppCd::parse(uint8_t aChar)
{

}
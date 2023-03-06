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
    uint32_t pos = findNextS((char*) aStartArg);

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

void TTermAppCd::exit()
{

}
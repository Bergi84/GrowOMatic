#ifndef TERMAPP_H_
#define TERMAPP_H_

#include "terminal.h"
#include "termPathMng.h"
#include <string.h>

class TTermApp
{
public:

protected:
    TTermApp();
    virtual ~TTermApp();

    // relaying functions becouse only parent class can access TTerminal
    inline void done()
    {   mTerm->exitApp();   }
    inline void putChar(uint8_t aChar)
    {   mTerm->putChar(aChar);  }
    inline void putString(const char *aStr, uint32_t len)
    {   mTerm->putString(aStr, len);    }
    inline void clrLine()
    {   mTerm->clrLine(); }
    inline void moveCursor(ctrlSym_e aDir, uint32_t aDist)
    {   mTerm->moveCursor(aDir, aDist);  }
    inline TTermPathMng* getPathMng() {return mTerm->mPathMng; };
    inline uint32_t findNextS(const char* str)
    {   return mTerm->findNextS(str);  };
    inline uint32_t findNextC(const char* str)
    {   return mTerm->findNextC(str);  };

    void printErr(errCode_T aEc);

protected:
    friend class TTerminal;

    const char* mKeyPhrase;

    virtual void start(uint8_t* aStartArg) = 0;
    virtual void parse(uint8_t aChar) = 0;

private:
    friend class TTerminal;

    TTerminal* mTerm;
    TTermApp* mNext;
};

#endif /* TERMAPP_H_*/
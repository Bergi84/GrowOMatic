#ifndef TERMAPP_H_
#define TERMAPP_H_

#include "terminal.h"

class TTermApp
{
public:

protected:
    TTermApp();

private:
    friend class TTerminal;

    TTerminal* mTerm;
    TTermApp* mNext;
    const char* mKeyPhrase;

    virtual void start(uint8_t* aStartArg);
    virtual void parse(uint8_t aChar);

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
};

#endif /* TERMAPP_H_*/
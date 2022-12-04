#ifndef GM_TERMINAL_H_
#define  GM_TERMINAL_H_

#include "uart.h"

class gm_terminal;

class gm_termApp
{
public:

protected:
    gm_termApp();

private:
    friend class gm_terminal;

    gm_termApp* mNext;
    const char* mKeyPhrase;
};

class gm_terminal 
{
public:
    void init(TUart* aUart);

private:
    TUart *mUart; 

    gm_termApp* mRootApp;

    static void rxCb(void* aArg);


};

#endif /*  GM_TERMINAL_H_ */
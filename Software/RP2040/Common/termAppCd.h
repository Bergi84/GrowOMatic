#ifndef TERMAPPCD_H_
#define TERMAPPCD_H_

#include "termApp.h"

class TTermAppCd : public TTermApp
{
public:
    TTermAppCd();
    virtual ~TTermAppCd();

private:
    static constexpr char mName[] = "cd"; 

    virtual void start(uint8_t* aStartArg);
    virtual void parse(uint8_t aChar);
    virtual void exit();
};

#endif /* TERMAPP_H_*/
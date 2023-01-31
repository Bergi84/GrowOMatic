#ifndef TERMAPPLS_H_
#define TERMAPPLS_H_

#include "termApp.h"

class TTermAppLs : public TTermApp
{
public:
    TTermAppLs();
    virtual ~TTermAppLs();

private:
    static constexpr char mName[] = "ls"; 

    virtual void start(uint8_t* aStartArg);
    virtual void parse(uint8_t aChar);
};

#endif /* TERMAPP_H_*/
#ifndef GM_TEMAPPPATHACCESS_H_
#define GM_TEMAPPPATHACCESS_H_

#include "termApp.h"

class gm_termAppPathAccess : public TTermApp
{
public:
    gm_termAppPathAccess();
    virtual ~gm_termAppPathAccess();

    virtual void start(uint8_t* aStartArg);
    virtual void parse(uint8_t aChar);
};

#endif
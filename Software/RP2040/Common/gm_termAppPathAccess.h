#ifndef GM_TEMAPPPATHACCESS_H_
#define GM_TEMAPPPATHACCESS_H_

#include "termApp.h"
#include "gm_termPathDefs.h"

class gm_termAppPathAccess : public TTermApp
{
public:
    gm_termAppPathAccess();
    virtual ~gm_termAppPathAccess();

    virtual void start(uint8_t* aStartArg);
    virtual void parse(uint8_t aChar);

private:
    static void writeCb (void* aArg, uint32_t* aVal, errCode_T aStatus);
    static void readCb (void* aArg, uint32_t* aVal, errCode_T aStatus);

    void readObj(pathObj_t* aObj);
    void writeObj(pathObj_t* aObj, uint32_t aVal);

    pathObj_t* mLastObj;
};

#endif
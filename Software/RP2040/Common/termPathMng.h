#ifndef TERMPATHMNG_H_
#define TERMPATHMNG_H_

#include "stdint.h"
#include "commonDefs.h"

#define TPM_PATH_MAX_LEN                64
#define TPM_FOLDER_MAX_LEN              16

class TTermApp;

class TTermPathMng
{
public:
    // returns written chars
    virtual uint32_t getSubPath(uint32_t aInd, char* aSubPath, uint32_t aSubPathLen, char* aPath = 0) = 0;

    virtual errCode_T setAktPath(char* aPath, uint32_t aPathLen = 0) = 0;

    // returns written chars
    virtual uint32_t getAktPath(char* aPath, uint32_t aPathLen) = 0;

    virtual ~TTermPathMng();

    TTermApp* mPathAccessApp;

protected:
    TTermPathMng();
};

#endif /* TERMPATHMNG_H_ */
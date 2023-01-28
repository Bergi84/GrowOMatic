#ifndef TERMPATHMNG_H_
#define TERMPATHMNG_H_

#include "stdint.h"


#define TPM_PATH_MAX_LEN                64
#define TPM_FOLDER_MAX_LEN              16

class TTermPathMng
{
public:
    virtual uint32_t getSubPathListLen(char* aPath = 0) = 0;
    virtual void getSubPath(uint32_t aInd, char* aSubPath, uint32_t aSubPathLen, char* aPath = 0) = 0;

    virtual void setAktPath(char* aPath, uint32_t aPathLen = 0) = 0;
    virtual void getAktPath(char* aPath, uint32_t aPathLen) = 0;

    virtual ~TTermPathMng();

protected:
    TTermPathMng();
};

#endif /* TERMPATHMNG_H_ */
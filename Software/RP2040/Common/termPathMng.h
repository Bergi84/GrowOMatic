#ifndef TERMPATHMNG_H_
#define TERMPATHMNG_H_

#include "stdint.h"


#define PATH_MAX_LEN                64

class TTermPathMng
{
public:
    virtual uint32_t getSubPathListLen() = 0;
    virtual uint8_t* getSubPath(uint32_t aInd) = 0;

    virtual void setAktPath(uint8_t* aPath, uint32_t aLen) = 0;
    virtual uint8_t* getAktPath() = 0;

    virtual ~TTermPathMng();

protected:
    TTermPathMng();
};

#endif /* TERMPATHMNG_H_ */
#ifndef GM_TERMPATHMNG_H_
#define GM_TERMPATHMNG_H_

#include "termPathMng.h"
#include "gm_busMaster.h"

class GM_termPathMng : public TTermPathMng
{
public:
    GM_termPathMng();

    void init(GM_busMaster* aBusMaster);

    virtual uint32_t getSubPathListLen();
    virtual uint8_t* getSubPath(uint32_t aInd);

    virtual void setAktPath(uint8_t* aPath, uint32_t aLen);
    virtual uint8_t* getAktPath();

private:
    GM_busMaster* mBM;

    struct
    {
        uint8_t bus;
        uint8_t adr;
        uint16_t ep;
        uint32_t uid;
        uint8_t pathStr[PATH_MAX_LEN];
    } mAktAdr;
    
};

#endif /* GM_TERMPATHMNG_H_ */
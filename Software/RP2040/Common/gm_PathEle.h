#ifndef GM_PATHELE_H_
#define GM_PATHELE_H_

#include "commonDefs.h"
#include "stdint.h"
#include "paraTable.h"
#include "gm_epLib.h"

class TPathEle {
private:
    typedef enum {
        POT_NONE,
        POT_FOLDER,
        POT_LOCREG,
        POT_REMREG
    } pathObjType_t;

    pathObjType_t mType;
    union {
        TParaTable::endpoint_t* loc;
        TEpBase* rem;
    } mEp;
    uint16_t mOffInd;

    TParaTable* mPt;

private:
    friend class GM_termPathMng;

    void init(TParaTable::endpoint_t* aEp, uint16_t aOffInd, TParaTable* aPt);
    void init(TEpBase* aEp, uint16_t aOffInd);
    void deInit();

public:
    TPathEle();

    bool isFolder();
    bool isFile();

    errCode_T getValue(void (*aCb) (void* aArg, uint32_t* aVal, errCode_T aStatus), void* aArg);
    errCode_T setValue(uint32_t aVal, void (*aCb) (void* aArg, uint32_t* aVal, errCode_T aStatus), void* aArg);

    uint32_t getPer();
    const char* getName();
};

#endif
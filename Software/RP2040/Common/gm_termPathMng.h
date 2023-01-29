#ifndef GM_TERMPATHMNG_H_
#define GM_TERMPATHMNG_H_

#include "termPathMng.h"
#include "gm_busMaster.h"
#include "paraTable.h"
#include "gm_termAppPathAccess.h"

class GM_termPathMng : public TTermPathMng
{
public:
    typedef enum {
        POT_NONE,
        POT_FOLDER,
        POT_LOCREG,
        POT_EPREG
    } pathObjType_t;

    typedef struct {
        pathObjType_t pot;
        void* objP;
        uint32_t ind;
    } pathObj_t;

    GM_termPathMng();
    virtual ~GM_termPathMng();

    void init(GM_busMaster* aBusMaster, TParaTable* aParaTable);

    // terminal path functions
    virtual uint32_t getSubPath(uint32_t aInd, char* aSubPath, uint32_t aSubPathLen, char* aPath = 0);

    virtual errCode_T setAktPath(char* aPath, uint32_t aPathLen = 0);
    virtual uint32_t getAktPath(char* aPath, uint32_t aPathLen);

    // applikation functions
    pathObj_t getPathObj(uint8_t* aPath);

private:
    GM_busMaster* mBM;
    TParaTable* mPT;

    gm_termAppPathAccess mPA;

    typedef enum pathType_e {
        PT_BUS,
        PT_LOC,
        PT_UID,
        PT_DEV,
        PT_INVALID,
        PT_ROOT,
    } pathType_t;

    static constexpr const char* cPathTypeName[]= {
        [PT_BUS] = "bus",
        [PT_LOC] = "loc",
        [PT_UID] = "uid",
        [PT_DEV] = "dev",
    };
    static constexpr const char* cSlaveStr = "slave";
    static const uint32_t cSlaveStrLen = sizeof(cPathTypeName)/sizeof(char*);

    typedef struct
    {
        pathType_t type;
        uint8_t bus;
        uint8_t adr;
        uint16_t baseInd;
        uint16_t offInd;
        uint32_t uid;
    } pathRes_t;
    pathRes_t mAktAdr;
    
    pathRes_t pathParse(char *aPathStr, uint32_t aStrLen = 0);
    uint32_t genPathString(pathRes_t *aPath, char* aStrBuf, uint32_t aBufLen);
    void parseEpRegName(pathRes_t *aPath, GM_device* aDev, char* aStrBuf, uint32_t aBufLen);
    void parseLocEpRegName(pathRes_t *aPath, char* aStrBuf, uint32_t aBufLen);
    TEpBase* parseEpName(GM_device* aDev, char* aPathStr, uint32_t aPathStrLen);
    TParaTable::endpoint_t* parseLocEpName(char* aPathStr, uint32_t aPathStrLen);
    uint16_t parseRegName(TEpBase* aEp, char* aPathStr, uint32_t aPathStrLen);
    uint16_t parseLocRegName(TParaTable::endpoint_t* aEp, char* aPathStr, uint32_t aPathStrLen);
    GM_device* parseUid(char* aPathStr, uint32_t aPathStrLen);
    GM_device* parseDevName(char* aPathStr, uint32_t aPathStrLen);

    uint32_t printSlaveName(uint32_t aSlaveInd, char* aStr, uint32_t aStrLen);
    uint32_t printUid(uint32_t aUid, char* aStr, uint32_t aStrLen);
};

#endif /* GM_TERMPATHMNG_H_ */
    #ifndef GM_TERMPATHDEFS_H_
    #define GM_TERMPATHDEFS_H_
    
    typedef enum {
        POT_NONE,
        POT_FOLDER,
        POT_LOCREG,
        POT_REMREG
    } pathObjType_t;

    typedef struct {
        pathObjType_t type;
        void* objP;
        uint16_t offInd;
    } pathObj_t;

    #endif
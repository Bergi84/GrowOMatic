#ifndef COMMONDEF_H_
#define COMMONDEF_H_

typedef enum
{
    EC_SUCCESS,
    EC_UNKNOWEN,
    EC_INVALID_DEVADR,
    EC_INVALID_REGADR,
    EC_QUEUE_FULL,
    EC_TIMEOUT,
    EC_INVALID_UID,
    EC_NOT_INIT,
    EC_OUT_OF_MEM,
    EC_INVALID_PATH,
    EC_PROTECTED,
    EC_INVALID_VALUE,
    EC_NOTAFOLDER,
    EC_NOTAFILE,
    EC_NOT_IMPLEMENTED,
    EC_ARB_ERR,
    EC_USER_ABORT
} errCode_T;

constexpr const char* CErrorDesc[] = {
    [EC_SUCCESS] = "no error",
    [EC_UNKNOWEN] = "unknowen error",
    [EC_INVALID_DEVADR] = "invalid device address",
    [EC_INVALID_REGADR] = "invalid register address",
    [EC_QUEUE_FULL] = "queue full",
    [EC_TIMEOUT] = "timeout",
    [EC_INVALID_UID] = "invalid uid",
    [EC_NOT_INIT] = "not initialised",
    [EC_OUT_OF_MEM] = "out of memory",
    [EC_INVALID_PATH] = "invalid path",
    [EC_PROTECTED] = "protected",
    [EC_INVALID_VALUE] = "invalid value",
    [EC_NOTAFOLDER] = "not a folder",
    [EC_NOTAFILE] = "not a file",
    [EC_NOT_IMPLEMENTED] = "not implemented",
    [EC_ARB_ERR] = "arbitation error",
    [EC_USER_ABORT] = "user abort"
};

#endif
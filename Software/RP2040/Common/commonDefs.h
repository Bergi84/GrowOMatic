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
    EC_INVALID_VALUE
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
    [EC_INVALID_VALUE] = "invalid value"
};

#endif
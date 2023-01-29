#ifndef COMMONDEF_H_
#define COMMONDEF_H_

typedef enum
{
    EC_SUCCESS,
    EC_INVALID_DEVADR,
    EC_INVALID_REGADR,
    EC_QUEUE_FULL,
    EC_TIMEOUT,
    EC_INVALID_UID,
    EC_NOT_INIT,
    EC_OUT_OF_MEM,
    EC_INVALID_PATH
} errCode_T;

#endif
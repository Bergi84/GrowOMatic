#ifndef RP_FLASH_H_
#define RP_FLASH_H_

#include "hardware/flash.h"
#include "storage.h"

class TFlash : public TStorage
{
private:
    uint32_t mOffset;
    uint32_t mSize;
    bool mSecCore;

public:
    // aOffset gives the start position in flash for the data storage sector
    // aSize is the size of the data storage sector, aSecCore signals that the second
    // core is running and has executed multicore_lockout_victim_init()
    void init(uint32_t aOffset, uint32_t aSize, bool aSecCore);

    virtual void store(bool (*aStoreDataCb)(void* aArg, uint32_t* aData, uint32_t aLen), void* aArg);
    virtual void load(bool (*aLoadDataCb)(void* aArg, uint32_t* aData, uint32_t aLen), void* aArg);
};

#endif /* RP_FLASH_H_ */
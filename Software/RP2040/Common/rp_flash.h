#ifndef RP_FLASH_H_
#define RP_FLASH_H_

#include "hardware/flash.h"
#include "storage.h"
#include "crc32.h"

class TFlash : public TStorage, public TCrc32
{
private:
    uint32_t mOffset;   // in Byte
    uint32_t mSize;     // in Byte
    bool mSecCore;

    bool checkCrc();

public:
    TFlash();

    // aOffset gives the start position in flash for the data storage sector
    // aSize is the size of the data storage sector, aSecCore signals that the second
    // core is running and has executed multicore_lockout_victim_init()
    void init(uint32_t aOffset, uint32_t aSize, bool aSecCore);

    virtual void store(uint32_t mSize, bool (*aStoreDataCb)(void* aArg, uint32_t* aData, uint32_t aLen), void* aArg);
    virtual void load(bool (*aLoadDataCb)(void* aArg, uint32_t* aData, uint32_t aLen), void* aArg);
    virtual void clear();
    
    virtual uint32_t getStoredSize();
    virtual uint32_t getMaxSize();
};

#endif /* RP_FLASH_H_ */
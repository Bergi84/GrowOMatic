#ifndef TSTORAGE_H_
#define TSTORAGE_H_

#include "stdint.h"

class TStorage
{
public:
    virtual ~TStorage();
    // starts a new storage cycle, the callback function must return true if
    // there are further data to save, aData is pointer to store the next piece of data
    // aLen is the size of the write buffer in words (uint32_t)
    virtual void store(uint32_t mSize, bool (*aStoreDataCb)(void* aArg, uint32_t* aData, uint32_t aLen), void* aArg) = 0;

    // starts a new load cycle, the callback function must return true if further 
    // data are wanted, aData points to the new piece of data
    // aLen is the size of the write buffer in words (uint32_t)
    virtual void load(bool (*aLoadDataCb)(void* aArg, uint32_t* aData, uint32_t aLen), void* aArg) = 0;

    virtual void clear() = 0;

    // returns size in words
    virtual uint32_t getStoredSize() = 0;
    
    // returns size in words
    virtual uint32_t getMaxSize() = 0;
};


#endif /* TSTORAGE_H_ */
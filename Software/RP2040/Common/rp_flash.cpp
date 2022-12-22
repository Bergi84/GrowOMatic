#include "rp_flash.h"
#include "pico/multicore.h"
#include <stdlib.h>

void TFlash::init(uint32_t aOffset, uint32_t aSize, bool aSecCore)
{
    if( aOffset%FLASH_SECTOR_SIZE || aSize%FLASH_SECTOR_SIZE || 
        aOffset + aSize > PICO_FLASH_SIZE_BYTES )
    {
        while(1);
    }
    mOffset = aOffset;
    mSize = aSize;
    mSecCore = aSecCore;
}

void TFlash::store(bool (*aStoreDataCb)(void* aArg, uint32_t* aData, uint32_t aLen), void* aArg)
{
    uint32_t* writeBuf = (uint32_t*) malloc(FLASH_PAGE_SIZE);
    if(writeBuf == 0)
        while(1);

    uint32_t status = save_and_disable_interrupts();
    if(mSecCore)
        multicore_lockout_start_blocking();

    flash_range_erase(mOffset, mSize);

    bool retValue = true;
    uint32_t writePos = 0;
    while(retValue && writePos < mSize)
    {
        retValue = aStoreDataCb(aArg, writeBuf, FLASH_PAGE_SIZE >> 2);
        flash_range_program(mOffset + writePos, (uint8_t*) writeBuf, FLASH_PAGE_SIZE);
        writePos += FLASH_PAGE_SIZE;
    }

    if(mSecCore)
        multicore_lockout_end_blocking();
    restore_interrupts(status);

    free(writeBuf);
}

void TFlash::load(bool (*aLoadDataCb)(void* aArg, uint32_t* aData, uint32_t aLen), void* aArg)
{
    aLoadDataCb(aArg, (uint32_t*) XIP_BASE + mOffset, mSize);
}
#include "rp_flash.h"
#include "pico/multicore.h"
#include <stdlib.h>

TFlash::TFlash() : TCrc32()
{

}

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

void TFlash::store(uint32_t aSize, bool (*aStoreDataCb)(void* aArg, uint32_t* aData, uint32_t aLen), void* aArg)
{
    uint32_t* writeBuf = (uint32_t*) malloc(FLASH_PAGE_SIZE);
    if(writeBuf == 0 || aSize > ((mSize >> 2) - 2))
        while(1);

    writeBuf[0] = aSize;
    uint32_t crc = 0;

    uint32_t status = save_and_disable_interrupts();
    if(mSecCore)
        multicore_lockout_start_blocking();

    flash_range_erase(mOffset, mSize);

    bool retValue = true;
    uint32_t writePos = 0;

    while(retValue && aSize > 0)
    {
        uint32_t aktSize;
        if(writePos == 0)
        {
            if(aSize < (FLASH_PAGE_SIZE >> 2) - 1)
            {
                aktSize = aSize;
            }
            else
            {
                aktSize = ((FLASH_PAGE_SIZE >> 2) - 1);
            }
            retValue = aStoreDataCb(aArg, &writeBuf[1], aktSize);
        }
        else
        {
            if(aSize < (FLASH_PAGE_SIZE >> 2))
            {
                aktSize = aSize;
            }
            else
            {
                aktSize = (FLASH_PAGE_SIZE >> 2);
            }
            retValue = aStoreDataCb(aArg, writeBuf, aktSize);
        }
        aSize -= aktSize;

        for(int i = 0; i < aktSize; i++)
            crc = crcCalc(crc, writeBuf[i]);

        if(aSize == 0)
        {
            writeBuf[aktSize] = crc;
        }
        
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
    if(checkCrc())
    {
        uint32_t* data = (uint32_t*) (XIP_BASE + mOffset);
        aLoadDataCb(aArg, &data[1], data[0]);
    }
}

uint32_t TFlash::getMaxSize()
{
    return (mSize >> 2) - 2;
}

uint32_t TFlash::getStoredSize()
{
    if(checkCrc())
    {
        uint32_t* data = (uint32_t*) (XIP_BASE + mOffset);

        return data[0];
    }
    return 0;
}

bool TFlash::checkCrc()
{
    uint32_t crc = 0;
    uint32_t* data = (uint32_t*) (XIP_BASE + mOffset);

    uint32_t size = data[0];

    if(size != 0 && size != 0xFFFFFFFF)
    {
        for( int i = 1; i <= size; i++)
        crc = crcCalc(crc, data[i]);
    }

    return (crc == data[size+1]);
}
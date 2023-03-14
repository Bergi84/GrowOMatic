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

void TFlash::deinit()
{
    mOffset = 0;
    mSize = 0;
    mSecCore = false;
}

void TFlash::store(uint32_t aSize, bool (*aStoreDataCb)(void* aArg, uint32_t* aData, uint32_t aLen), void* aArg)
{
    if(aStoreDataCb == 0 || mSize == 0)
        return;

    uint32_t* writeBuf = (uint32_t*) malloc(FLASH_PAGE_SIZE);
    for(int i = 0; i < FLASH_PAGE_SIZE >> 2; i++)
        writeBuf[i] = 0;

    if(writeBuf == 0 || aSize > ((mSize >> 2) - 2))
        while(1);

    writeBuf[0] = aSize;
    uint32_t crc = 0;

    uint32_t status = save_and_disable_interrupts();
    if(mSecCore)
        // bug in SDK 1.4.0
        // multicore_lockout_start_blocking();
        multicore_lockout_start_timeout_us((uint64_t)356*24*60*60*1000*1000);

    flash_range_erase(mOffset, mSize);

    bool retValue = true;
    uint32_t writePos = 0;

    aSize++;    // we need addition uint32_t for crc
    while((retValue || aSize == 1) && aSize > 0)
    {
        uint32_t aktSize;
        if(writePos == 0)
        {
            // first page holds also the length at address offset 0
            if(aSize <= (FLASH_PAGE_SIZE >> 2) - 1)
            {
                aktSize = aSize - 1;
            }
            else
            {
                aktSize = ((FLASH_PAGE_SIZE >> 2) - 1);
            }
            if(aktSize > 0)
                retValue = aStoreDataCb(aArg, &writeBuf[1], aktSize);

            for(int i = 0; i < aktSize + 1; i++)
                crc = crcCalc(crc, writeBuf[i]);
        }
        else
        {
            if(aSize <= (FLASH_PAGE_SIZE >> 2))
            {
                aktSize = aSize - 1;
            }
            else
            {
                aktSize = (FLASH_PAGE_SIZE >> 2);
            }
            if(aktSize > 0)
                retValue = aStoreDataCb(aArg, writeBuf, aktSize);

            for(int i = 0; i < aktSize + 0; i++)
                crc = crcCalc(crc, writeBuf[i]);
        }
        aSize -= aktSize;



        if(aSize == 1)
        {
            aSize--;
            if(writePos == 0)
                writeBuf[aktSize + 1] = crc;
            else
                writeBuf[aktSize] = crc;
        }
        
        flash_range_program(mOffset + writePos, (uint8_t*) writeBuf, FLASH_PAGE_SIZE);
        writePos += FLASH_PAGE_SIZE;
    }

    if(mSecCore)
        // multicore_lockout_end_blocking();
        multicore_lockout_end_timeout_us((uint64_t)356*24*60*60*1000*1000);
    restore_interrupts(status);

    free(writeBuf);
}

void TFlash::load(bool (*aLoadDataCb)(void* aArg, uint32_t* aData, uint32_t aLen), void* aArg)
{
    if(aLoadDataCb == 0 || mSize == 0)
        return;

    if(checkCrc())
    {
        uint32_t* data = (uint32_t*) (XIP_BASE + mOffset);
        aLoadDataCb(aArg, &data[1], data[0]);
    }
}

void TFlash::clear()
{
    uint32_t status = save_and_disable_interrupts();
    if(mSecCore)
    // bug in SDK 1.4.0
    // multicore_lockout_start_blocking();
        multicore_lockout_start_timeout_us((uint64_t)356*24*60*60*1000*1000);

    flash_range_erase(mOffset, mSize);

    if(mSecCore)
    //    multicore_lockout_end_blocking();
        multicore_lockout_end_timeout_us((uint64_t)356*24*60*60*1000*1000);
    restore_interrupts(status);
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
    uint32_t storedCrc = data[size+1];

    if(size != 0 && size < mSize)
    {
        // without DMB and with enabled optimization the compiler 
        // try to access the location data[size+1] also if size is an invalid value  
        __dmb();    
        for( int i = 0; i <= size; i++)
            crc = crcCalc(crc, data[i]);
        return (crc == storedCrc);
    }
    else
    {
        return false;
    }
}

void TFlash::storePage(uint32_t aOffset, uint8_t aData[FLASH_PAGE_SIZE], bool aSecCore)
{
    uint32_t status = save_and_disable_interrupts();
    if(aSecCore)
    // bug in SDK 1.4.0
    // multicore_lockout_start_blocking();
        multicore_lockout_start_timeout_us((uint64_t)356*24*60*60*1000*1000);

    flash_range_program(aOffset, aData, FLASH_PAGE_SIZE);

    if(aSecCore)
    //    multicore_lockout_end_blocking();
        multicore_lockout_end_timeout_us((uint64_t)356*24*60*60*1000*1000);
    restore_interrupts(status);
}

void TFlash::eraseSektors(uint32_t aOffset, uint32_t aLen, bool aSecCore)
{
    uint32_t status = save_and_disable_interrupts();
    if(aSecCore)
    // bug in SDK 1.4.0
    // multicore_lockout_start_blocking();
        multicore_lockout_start_timeout_us((uint64_t)356*24*60*60*1000*1000);

    flash_range_erase(aOffset, aLen);

    if(aSecCore)
    //    multicore_lockout_end_blocking();
        multicore_lockout_end_timeout_us((uint64_t)356*24*60*60*1000*1000);
    restore_interrupts(status);
}
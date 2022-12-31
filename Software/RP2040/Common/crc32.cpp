#include "crc32.h"

bool TCrc32::mCrcTabInit = false;
uint32_t TCrc32::mCrcTab[256];

uint32_t TCrc32::crcCalc(uint32_t aCrc, uint8_t aByte)
{
    return (aCrc >> 8) ^ mCrcTab[((uint8_t)aCrc) ^ aByte];
}

uint32_t TCrc32::crcCalc(uint32_t aCrc, uint32_t aWord)
{
    uint32_t crc = (aCrc >> 8) ^ mCrcTab[((uint8_t)aCrc) ^ ((aWord >> 0) & 0x000000FF)];
    crc = (crc >> 8) ^ mCrcTab[((uint8_t)crc) ^ ((aWord >> 8) & 0x000000FF)];
    crc = (crc >> 8) ^ mCrcTab[((uint8_t)crc) ^ ((aWord >> 16) & 0x000000FF)];
    return (crc >> 8) ^ mCrcTab[((uint8_t)crc) ^ ((aWord >> 24) & 0x000000FF)];
}

void TCrc32::crcInitTab()
{
    for (uint32_t i = 0; i < 256; i++) {

        uint32_t crc = i;

        for (uint32_t j = 0; j < 8; j++) {

            if ( crc & 0x00000001 ) crc = ( crc >> 1 ) ^ TCrc32::mCrcPoly;
            else                     crc =   crc >> 1;
        }

        TCrc32::mCrcTab[i] = crc;
    }
}

TCrc32::TCrc32()
{
    if(!mCrcTabInit)
    {
        crcInitTab();
    }
}
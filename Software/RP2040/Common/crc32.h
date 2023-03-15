#ifndef CRC32_H_
#define CRC32_H_

#include <stdint.h>
#include "pico/platform.h"

class TCrc32
{
public:
    static constexpr uint32_t mCrcPoly = 0xEDB88320;

    static uint32_t __time_critical_func(crcCalc)(uint32_t aCrc, uint8_t aByte);
    static uint32_t __time_critical_func(crcCalc)(uint32_t aCrc, uint32_t aWord);

    TCrc32();

private:
    static bool mCrcTabInit;
    static uint32_t mCrcTab[256];

    static void crcInitTab();
};

#endif /* CRC32_H_ */

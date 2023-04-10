#ifndef IRQVENEER_H_
#define IRQVENEER_H_

#include "stdint.h"

typedef struct
{
    uint16_t inst0;
    uint16_t inst1;
    uint16_t inst2;
    uint16_t rsvd;

    void* arg;
    void (*handler)(void* aArg);

    typedef void (*irqHandler_t)();
    irqHandler_t getFunc() {
        return (irqHandler_t) (((uint32_t) this) | 0x1);
    };

    // program counter relative thumb load instruction
    static inline uint16_t LDRliteral(uint32_t reg, uint32_t immerdate)
    {
        return 0x4800 + (reg << 8) + (immerdate >> 2);
    };

    // branch to address in register
    static inline uint16_t BX(uint32_t reg)
    {
        return 0x4700 + (reg << 3);
    };

    void init(void* aArg,  void (*aHandler)(void* aArg))
    {
        arg = aArg;
        handler = aHandler;

        inst0 = LDRliteral(0, 4);  // load func.arg to r0
        inst1 = LDRliteral(1, 8);  // load func.handler to r1
        inst2 = BX(1);             // brunch to func.handler   
    };
} irqVeneer_t;

#endif
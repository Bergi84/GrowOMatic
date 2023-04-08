#include "irqVeneer.h"

// program counter relative thumb load instruction
uint16_t LDRliteral(uint32_t reg, uint32_t immerdate)
{
    return 0x4800 + (reg << 8) + (immerdate >> 2);
}

// increment register by immerdate
uint16_t ADDS(uint32_t reg, uint32_t immerdate)
{
    return 0x6000 + reg << 8 + immerdate;
}

// branch to address in register
uint16_t BX(uint32_t reg)
{
    return 0x4700 + (reg << 3);
}

irqFunc_t genIrqVeneer(void* aArg,  void (*aHandler)(void* aArg))
{
    irqFunc_t func;

    func.arg = aArg;
    func.handler = aHandler;

    func.inst0 = LDRliteral(0, 4);  // load func.arg to r0
    func.inst1 = LDRliteral(1, 8);  // load func.handler to r1
    func.inst2 = BX(1);             // brunch to func.handler   

    return func;
}
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

    typedef void (*irqHandler_t)(void*);
    irqHandler_t getFunc() {
        return (irqHandler_t) (((uint32_t) this) | 0x1);
    }
} irqFunc_t;

irqFunc_t genIrqVeneer(void* aArg,  void (*aHandler)(void* aArg));

#endif
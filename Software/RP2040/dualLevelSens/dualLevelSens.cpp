#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/sync.h"
#include "dualLevelSens.h"
#include "capSens.h"
#include "sequencer_armm0.h"
#include "uart.h"
#include "gm_busSlave.h"
#include "paraTable.h"
#include "pico/unique_id.h"
#include "terminal.h"

TCapSens<gpio_capSens_chNo> gCapSens;
TSequencer gSeq, gSeq_c1;
THwUart gUart0;
THwUart gUart1;
TUsbUart gUartTerm;
GM_busSlave gSlave;
TParaTable gParaTable;
TTerminal gTerm;

void capSensIrqHandler()
{
    gCapSens.irqHandler();
}

void uart0IrqHandler()
{
    gUart0.irqHandler();
}

void uart1IrqHandler()
{
    gUart1.irqHandler();
}

void uartTermIrqHandler()
{
    gUartTerm.irqHandler();
}

void idle(void* aArg)
{
//    __wfi();
}

extern uint32_t __StackOneTop;

#define STATUP_FLAG 0x0000cafe

void main_c1() 
{
    gSeq_c1.init(&__StackOneTop, PICO_STACK_SIZE);
    gSeq_c1.setIdleFunc(idle, NULL);

    gCapSens.init(pio0, gpio_capSensExc_base, gpio_capSens_base, 0x000007FF, 62500, 125);
    gCapSens.setIrqHandler(capSensIrqHandler);
    gCapSens.enable();

    gSeq_c1.startIdle();
}

extern uint32_t __StackTop;

int main() 
{
    multicore_launch_core1(main_c1);

    gSeq.init(&__StackTop, PICO_STACK_SIZE);
    gSeq.setIdleFunc(idle, NULL);

    gUart0.init(uart0, gpio_uart0_tx, gpio_uart0_rx);
    gUart0.setIrqHandler(uart0IrqHandler);

    gUart1.init(uart1, gpio_uart1_tx, gpio_uart1_rx);
    gUart1.setIrqHandler(uart1IrqHandler);

    gUartTerm.init(&gSeq);
    gUartTerm.setIrqHandler(uartTermIrqHandler);
    gTerm.init(&gUartTerm, &gSeq);

    pico_unique_board_id_t uId;
    pico_get_unique_board_id(&uId);

    gParaTable.init(*((uint32_t*) uId.id), DT_DUAL_LEVEL_SENSOR);

    gSlave.init(&gUart0, &gUart1, &gParaTable, &gSeq);

    gSeq.startIdle();
}

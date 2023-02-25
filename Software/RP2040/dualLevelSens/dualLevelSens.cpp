#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/sync.h"
#include "dualLevelSens.h"
#include "capSens.h"
#include "sequencer_armm0.h"
#include "rp_uart.h"
#include "paraTable.h"
#include "gm_system.h"
#include "pico/unique_id.h"
#include "terminal.h"
#include "gm_termPathMng.h"
#include "rp_flash.h"
#include "gm_bus.h"

TCapSens<gpio_capSens_chNo> gCapSens;
TSequencer gSeq, gSeq_c1;
THwUart gUart0;
THwUart gUart1;
TUsbUart gUartTerm;
GM_bus gBus;
TParaTable gParaTable;
TSystem gSystem;
TTerminal gTerm;
GM_termPathMng gPathMng;
TFlash gTableStorage;

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


void main_c1() 
{
    // core1 lockout important in case of flash write or erase operations
    // if the second core runs only from ram the lockout is not needed
#if (PICO_COPY_TO_RAM == 0)
    multicore_lockout_victim_init();  
    irq_set_priority(SIO_IRQ_PROC1, PICO_HIGHEST_IRQ_PRIORITY);  
#endif

    gSeq_c1.init(&__StackOneTop, PICO_STACK_SIZE);
    gSeq_c1.setIdleFunc(idle, NULL);

    
    gCapSens.init(pio0, gpio_capSensExc_base, gpio_capSens_base, 0x000007FF /*0x3FF800*/, 62500, 125);
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

    pico_unique_board_id_t uId;
    pico_get_unique_board_id(&uId);
    
#if (PICO_COPY_TO_RAM == 0)
    gTableStorage.init(PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE, true);
#else
    gTableStorage.init(PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE, false);
#endif

    gParaTable.init(&gTableStorage);
    gSystem.init(*((uint32_t*) uId.id), DT_DUAL_LEVEL_SENSOR, &gParaTable);
    gSystem.setSysLed(gpio_systemLed);

    TUart* uartList[] = {&gUart0,  &gUart1};

    gBus.init(uartList, sizeof(uartList)/sizeof(TUart*), &gSeq, &gParaTable);

    gPathMng.init(&gBus, &gParaTable);

    gParaTable.loadPara();
    
    gTerm.init(&gUartTerm, &gSeq, &gPathMng);

    gSeq.startIdle();
}

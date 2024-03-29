#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/sync.h"
#include "uniFw.h"
#include "sequencer_armm0.h"
#include "rp_uart.h"
#include "paraTable.h"
#include "gm_system.h"
#include "pico/unique_id.h"
#include "terminal.h"
#include "gm_termPathMng.h"
#include "rp_flash.h"
#include "gm_bus.h"
#include "rp_debug.h"
#include "gm_dualCapSens.h"
#include "gm_termMonitor.h"
#include "rp_timerServer.h"
#include "gm_pumpCon.h"
#include "irqVeneer.h"

// objekts used for each configutation
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
gm_termMonitor gTermMonitor;
TTimerServer gTimeServer;
TTimerServer gTimeServer_c1;

// pointer to objects which are only instantiated if needed
GM_pumpCon* gPumpCon;
GM_dualCapSense* gDualCapSens;

void idle(void* aArg)
{
//    __wfi();
}

extern uint32_t __StackOneTop;

void main_c1() 
{
    // core1 lockout important in case of flash write or erase operations
    multicore_lockout_victim_init();  
    irq_set_priority(SIO_IRQ_PROC1, PICO_HIGHEST_IRQ_PRIORITY);  

    gSeq_c1.init(&__StackOneTop, PICO_STACK_SIZE);
    gSeq_c1.setIdleFunc(idle, NULL);

    gTimeServer_c1.init();

    gSeq_c1.startIdle();
}

extern uint32_t __StackTop;

int main() 
{
    gSeq.init(&__StackTop, PICO_STACK_SIZE);
    gSeq.setIdleFunc(idle, NULL);

    gTimeServer.init();

    multicore_launch_core1(main_c1);

    pico_unique_board_id_t uId;
    pico_get_unique_board_id(&uId);
    
    gTableStorage.init(FLASH_TYPE_STORAGE, FLASH_TYPE_STORAGE_SIZE);

    gParaTable.init(&gTableStorage);
    gSystem.init(*((uint32_t*) &uId.id[4]), &gParaTable, &gTimeServer);

    gParaTable.loadPara();

    uint32_t val;
    gParaTable.getPara(CSystemBaseRegAdr + TEpSysDefs::PARA_TYPE, &val);

    if(val != DT_INVALID)
    {
        gTableStorage.init(FLASH_PT_STORAGE, FLASH_PT_STORAGE_SIZE);
    }

    switch(val)
    {
        case DT_DUAL_LEVEL_SENSOR:
            {
                gUart0.init(uart0, gpio_dls_uart0_tx, gpio_dls_uart0_rx);
                gUart1.init(uart1, gpio_dls_uart1_tx, gpio_dls_uart1_rx);

                gSystem.setSysLed(gpio_dls_systemLed);

                TUart* uartList[] = {&gUart0,  &gUart1};
                gBus.init(uartList, sizeof(uartList)/sizeof(TUart*), &gSeq, &gTimeServer, &gParaTable);

                gDualCapSens = new GM_dualCapSense();
                gDualCapSens->init(&gParaTable, CDefaultBaseRegAdr, &gSeq_c1);

                gPathMng.init(&gBus, &gParaTable);
            }
            break;

        case DT_PICO_BOARD:
            {
                gDebug.init((1 << gpio_pb_dbg1) | (1 << gpio_pb_dbg2) | (1 << gpio_pb_dbg3) | (1 << gpio_pb_dbg4) | (1 << gpio_pb_dbg5) | (1 << gpio_pb_dbg6) | (1 << gpio_pb_dbg7) | (1 << gpio_pb_dbg8));

                gUart0.init(uart0, gpio_pb_uart0_tx, gpio_pb_uart0_rx);
                gUart1.init(uart1, gpio_pb_uart1_tx, gpio_pb_uart1_rx);

                gSystem.setSysLed(gpio_pb_systemLed);

                TUart* uartList[] = {&gUart0,  &gUart1};
                gBus.init(uartList, sizeof(uartList)/sizeof(TUart*), &gSeq, &gTimeServer, &gParaTable);   

                gPathMng.init(&gBus, &gParaTable);
            }
            break;

        case DT_PUMP_CON:
            {
                gUart0.init(uart0, gpio_pc_uart0_tx, gpio_pc_uart0_rx);
                gUart1.init(uart1, gpio_pc_uart1_tx, gpio_pc_uart1_rx);

                gSystem.setSysLed(gpio_pc_systemLed);

                TUart* uartList[] = {&gUart0,  &gUart1};
                gBus.init(uartList, sizeof(uartList)/sizeof(TUart*), &gSeq, &gTimeServer, &gParaTable);

                gPumpCon = new GM_pumpCon();
                gPumpCon->init(&gParaTable, &gTimeServer_c1, &gSeq_c1);

                gPathMng.init(&gBus, &gParaTable);
            }

        default:
            gPathMng.init(0, &gParaTable);
            break;
    }

    if(val != DT_INVALID)
    {
        gParaTable.loadPara();
    }

    gUartTerm.init(&gSeq);
    
    gTerm.init(&gUartTerm, &gSeq, &gPathMng);

    gTermMonitor.init(&gSeq, &gTimeServer);
    gTerm.addApp(&gTermMonitor);

    gSeq.startIdle();
}

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/sync.h"
#include "uniFw.h"
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
#include "rp_debug.h"
#include "gm_dualCapSens.h"
#include "gm_termMonitor.h"

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

// configutation depended objects
GM_dualCapSense* gDualCapSens;


// interrupt wrappers vor inline interrupt handlers
void __time_critical_func(uartTermIrqHandler)()
{
    gUartTerm.irqHandler();
}

void __time_critical_func(uart0IrqHandler)()
{
    gUart0.irqHandler();
}

void __time_critical_func(uart1IrqHandler)()
{
    gUart1.irqHandler();
}

void __time_critical_func(dualCapSensIrqHandler)()
{
    gDualCapSens->irqHandler();
}

typedef struct {
    void (*irqHandler)();
    void (*irqInstaller)(void* aArg, void (*aIrqHandler)());
    void* installerArg;
    volatile bool done;
} irqInstallPara_t;

void irqInstaller(void* aArg)
{
    irqInstallPara_t* para = (irqInstallPara_t*) aArg;

    para->irqInstaller(para->installerArg, para->irqHandler);

    para->done = true;
}

void installIrqOnC1(void (*aIrqHandler)(), void (*aIrqInstaller)(void* aArg, void (*aIrqHandler)()), void* aArg )
{
    irqInstallPara_t irqInstallPara;

    irqInstallPara.done = false;
    irqInstallPara.irqHandler = aIrqHandler;
    irqInstallPara.irqInstaller = aIrqInstaller;
    irqInstallPara.installerArg = aArg;

    uint8_t seqId;
    gSeq_c1.addTask(seqId, irqInstaller, &irqInstallPara);
    gSeq_c1.queueTask(seqId);

    while(irqInstallPara.done == false);
    gSeq_c1.delTask(seqId);
}

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

    gSeq_c1.startIdle();
}

extern uint32_t __StackTop;

int main() 
{
    multicore_launch_core1(main_c1);

    gSeq.init(&__StackTop, PICO_STACK_SIZE);
    gSeq.setIdleFunc(idle, NULL);

    pico_unique_board_id_t uId;
    pico_get_unique_board_id(&uId);
    
    gTableStorage.init(FLASH_TYPE_STORAGE, FLASH_TYPE_STORAGE_SIZE);

    gParaTable.init(&gTableStorage);
    gSystem.init(*((uint32_t*) &uId.id[4]), &gParaTable);

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
                gUart0.setIrqHandler(uart0IrqHandler);

                gUart1.init(uart1, gpio_dls_uart1_tx, gpio_dls_uart1_rx);
                gUart1.setIrqHandler(uart1IrqHandler);

                gSystem.setSysLed(gpio_dls_systemLed);

                TUart* uartList[] = {&gUart0,  &gUart1};
                gBus.init(uartList, sizeof(uartList)/sizeof(TUart*), &gSeq, &gParaTable);
                gDualCapSens = new GM_dualCapSense();
                gDualCapSens->init(&gParaTable);
                installIrqOnC1(dualCapSensIrqHandler, gDualCapSens->setIrqHandler, gDualCapSens);

                gPathMng.init(&gBus, &gParaTable);
            }
            break;

        case DT_PICO_BOARD:
            {
                gDebug.init((1 << gpio_pb_dbg1) | (1 << gpio_pb_dbg2) | (1 << gpio_pb_dbg3) | (1 << gpio_pb_dbg4));

                gUart0.init(uart0, gpio_pb_uart0_tx, gpio_pb_uart0_rx);
                gUart0.setIrqHandler(uart0IrqHandler);

                gUart1.init(uart1, gpio_pb_uart1_tx, gpio_pb_uart1_rx);
                gUart1.setIrqHandler(uart1IrqHandler);

                gSystem.setSysLed(gpio_pb_systemLed);

                TUart* uartList[] = {&gUart0,  &gUart1};
                gBus.init(uartList, sizeof(uartList)/sizeof(TUart*), &gSeq, &gParaTable);

                gPathMng.init(&gBus, &gParaTable);
            }
            break;

        default:
            gPathMng.init(0, &gParaTable);
            break;
    }

    if(val != DT_INVALID)
    {
        gParaTable.loadPara();
    }

    gUartTerm.init(&gSeq);
    gUartTerm.setIrqHandler(uartTermIrqHandler);
    
    gTerm.init(&gUartTerm, &gSeq, &gPathMng);

    gTermMonitor.init(&gSeq);
    gTerm.addApp(&gTermMonitor);

    gSeq.startIdle();
}

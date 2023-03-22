
#include "hardware/flash.h"
#include "gm_board.h"

#define FLASH_TYPE_STORAGE        (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)
#define FLASH_TYPE_STORAGE_SIZE   FLASH_SECTOR_SIZE

#define FLASH_PT_STORAGE          (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE*3)
#define FLASH_PT_STORAGE_SIZE     (FLASH_SECTOR_SIZE*2)

#define FLASH_FW_BUFFER           (PICO_FLASH_SIZE_BYTES >> 1)
#define FLASH_FW_BUFFER_SIZE      ((PICO_FLASH_SIZE_BYTES >> 1) - FLASH_SECTOR_SIZE*3)

// pico board defines

#define gpio_pb_usbPresent         24
#define gpio_pb_systemLed          25
#define gpio_pb_uart0_rx           13
#define gpio_pb_uart0_tx           12
#define gpio_pb_uart1_rx           5
#define gpio_pb_uart1_tx           4

#define gpio_pb_dbg1                6
#define gpio_pb_dbg2                7
#define gpio_pb_dbg3                8
#define gpio_pb_dbg4                9
#define gpio_pb_dbg5                10
#define gpio_pb_dbg6                11
#define gpio_pb_dbg7                14
#define gpio_pb_dbg8                15

// dual level sensor defines

#define gpio_dls_usbPresent         27
#define gpio_dls_systemLed          26
#define gpio_dls_uart0_rx           29
#define gpio_dls_uart0_tx           28
#define gpio_dls_uart1_rx           25
#define gpio_dls_uart1_tx           24

// pump controller

#define gpio_pc_uart0_tx            0
#define gpio_pc_uart0_rx            1
#define gpio_pc_uart1_tx            4
#define gpio_pc_uart1_rx            5    
#define gpio_pc_systemLed           6
#define gpio_pc_spiEn               8
#define gpio_pc_spiCs               9
#define gpio_pc_spiClk              10
#define gpio_pc_spiD                11
#define gpio_pc_pwmP0               12
#define gpio_pc_enP0                13
#define gpio_pc_pwmP1               14
#define gpio_pc_enP1                15
#define gpio_pc_iRefP0              16
#define gpio_pc_iRefP1              17
#define gpio_pc_usbPresent          18
#define gpio_pc_flowPuls1           21
#define gpio_pc_flowPuls0           23
#define gpio_pc_ledDim              24
#define gpio_pc_iSens0              26
#define gpio_pc_iSens1              27
#define gpio_pc_leakSens            29   

// sensor node

#define gpio_sn_spiCs               1
#define gpio_sn_spiSck              2
#define gpio_sn_spiSdi              3
#define gpio_sn_spiSdo              4
#define gpio_sn_phExc               5
#define gpio_sn_ecExcP              6
#define gpio_sn_ecExcN              7
#define gpio_sn_uart1_tx            8
#define gpio_sn_uart1_rx            9
#define gpio_sn_systemLed           10
#define gpio_sn_uart0_tx            16
#define gpio_sn_uart0_rx            17
#define gpio_sn_usbPresent          18
#define gpio_sn_sda                 24
#define gpio_sn_scl                 25
#define gpio_sn_din0                28
#define gpio_sn_din1                29
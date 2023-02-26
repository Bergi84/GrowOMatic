
#include "hardware/flash.h"
#include "board_uniFwBoard.h"

#define FLASH_TYPE_STORAGE        (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)
#define FLASH_TYPE_STORAGE_SIZE   FLASH_SECTOR_SIZE

#define FLASH_PT_STORAGE          (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE*3)
#define FLASH_PT_STORAGE_SIZE     FLASH_SECTOR_SIZE*2

// pico board defines

#define gpio_pb_usbPresent         24
#define gpio_pb_systemLed          25
#define gpio_pb_uart0_rx           1
#define gpio_pb_uart0_tx           0
#define gpio_pb_uart1_rx           5
#define gpio_pb_uart1_tx           4

#define gpio_pb_dbg1                6
#define gpio_pb_dbg2                7
#define gpio_pb_dbg3                8
#define gpio_pb_dbg4                9

// dual level sensor defines

#define gpio_dls_usbPresent         27
#define gpio_dls_systemLed          26
#define gpio_dls_uart0_rx           29
#define gpio_dls_uart0_tx           28
#define gpio_dls_uart1_rx           25
#define gpio_dls_uart1_tx           24
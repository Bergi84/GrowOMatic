add_executable(uniFw)

set(PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_LIST_DIR})
set(PICO_BOARD uniFw)

target_sources(uniFw PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/uniFw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/sequencer_armm0.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/uart.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/rp_uart.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/gm_busSlave.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/paraTable.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/gm_busMaster.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/gm_busDefs.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/gm_bus.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/gm_device.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/gm_epLib.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/tusb_discriptors.c
    ${CMAKE_CURRENT_LIST_DIR}/../common/terminal.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/termApp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/termPathMng.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/gm_termPathMng.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/storage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/rp_flash.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/crc32.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/gm_termAppPathAccess.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/termAppCd.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/termAppLs.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/gm_PathEle.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/gm_system.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/rp_debug.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/gm_dualCapSens.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/gm_termMonitor.cpp
)	
		
target_include_directories(uniFw PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../common
)	
		
target_link_libraries(uniFw PUBLIC
    pico_stdlib 
    hardware_pio 
    hardware_irq
    hardware_flash 
    hardware_watchdog
    pico_multicore 
    pico_unique_id 
    tinyusb_device 
    tinyusb_board
    )

target_compile_definitions(uniFw PUBLIC
    PICO_STACK_SIZE=0x1000
)

pico_generate_pio_header(uniFw ${CMAKE_CURRENT_LIST_DIR}/../common/capSens.pio)

# exectue from ram
# pico_set_binary_type(uniFw copy_to_ram)

# pico_enable_stdio_usb(uniFw 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(uniFw)

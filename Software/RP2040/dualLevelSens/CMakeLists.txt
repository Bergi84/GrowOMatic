add_executable(dualLevelSens)

set(PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_LIST_DIR})
set(PICO_BOARD board_dualLevelSens)

target_sources(dualLevelSens PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/dualLevelSens.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/sequencer_armm0.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/uart.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/rp_uart.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/gm_busSlave.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/paraTable.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/gm_busMaster.cpp
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
)	
		
target_include_directories(dualLevelSens PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../common
)	
		
target_link_libraries(dualLevelSens PUBLIC
    pico_stdlib 
    hardware_pio 
    hardware_irq
    hardware_flash 
    pico_multicore 
    pico_unique_id 
    tinyusb_device 
    tinyusb_board
    )

target_compile_definitions(dualLevelSens PUBLIC
    PICO_STACK_SIZE=0x1000
)

pico_generate_pio_header(dualLevelSens ${CMAKE_CURRENT_LIST_DIR}/../common/capSens.pio)

# exectue from ram
# pico_set_binary_type(dualLevelSens copy_to_ram)

pico_enable_stdio_usb(dualLevelSens 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(dualLevelSens)

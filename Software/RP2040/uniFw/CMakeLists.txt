add_executable(uniFw)

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
    ${CMAKE_CURRENT_LIST_DIR}/../common/rp_capSens.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/gm_dualCapSens.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/gm_termMonitor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/rp_timerServer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/gm_pumpCon.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/gm_stepperPhCon.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/rp_adc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/rp_dmaIrqMng.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/gm_dcMotorCon.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/rp_pwmCapture.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/gm_leakSensor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/gm_pulsSensor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/gm_digitalIn.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/gm_ledCon.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/../common/rp_i2cMng.cpp
)	
		
target_include_directories(uniFw PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../common
    ${CMAKE_CURRENT_LIST_DIR}/../boards
)	
		
target_link_libraries(uniFw PUBLIC
    pico_stdlib 
    hardware_pio 
    hardware_irq
    hardware_flash 
    hardware_watchdog
    hardware_adc
    hardware_dma
    hardware_pwm
    hardware_clocks
    hardware_i2c
    pico_multicore 
    pico_unique_id 
    tinyusb_device 
    tinyusb_board
    )

target_compile_definitions(uniFw PUBLIC
    PICO_STACK_SIZE=0x1000
)

pico_generate_pio_header(uniFw ${CMAKE_CURRENT_LIST_DIR}/../common/rp_capSens.pio)
pico_generate_pio_header(uniFw ${CMAKE_CURRENT_LIST_DIR}/../common/rp_ioExpander.pio)
pico_generate_pio_header(uniFw ${CMAKE_CURRENT_LIST_DIR}/../common/rp_pwmCapture.pio)

# exectue from ram
# pico_set_binary_type(uniFw copy_to_ram)

# pico_enable_stdio_usb(uniFw 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(uniFw)

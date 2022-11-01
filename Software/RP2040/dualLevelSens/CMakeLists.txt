add_executable(dualLevelSens)

set(PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_LIST_DIR})
set(PICO_BOARD board_dualLevelSens)

target_sources(dualLevelSens PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/dualLevelSens.cpp
    )	
		
target_include_directories(dualLevelSens PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../common
    )	
		
target_link_libraries(dualLevelSens pico_stdlib hardware_pio hardware_irq)

pico_generate_pio_header(dualLevelSens ${CMAKE_CURRENT_LIST_DIR}/../common/capSens.pio)

# exectue from ram
# pico_set_binary_type(dualLevelSens copy_to_ram)

pico_enable_stdio_usb(dualLevelSens 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(dualLevelSens)

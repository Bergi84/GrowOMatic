add_executable(testSequencer)

set(PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_LIST_DIR})
set(PICO_BOARD board_testSequencer)

target_sources(testSequencer PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/testSequencer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/sequencer_armm0.cpp
    )	
		
target_include_directories(testSequencer PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../common
    )	
		
target_link_libraries(testSequencer pico_stdlib)

# pico_generate_pio_header(testSequencer ${CMAKE_CURRENT_LIST_DIR}/capTouchSens.pio)

# exectue from ram
# pico_set_binary_type(testSequencer copy_to_ram)

pico_enable_stdio_usb(testSequencer 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(testSequencer)

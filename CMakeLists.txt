cmake_minimum_required(VERSION 3.18)

set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/external/pico-sdk" CACHE PATH "Pico SDK path")
include("${PICO_SDK_PATH}/external/pico_sdk_import.cmake")

project(wup_1028_oc C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

pico_sdk_init()

set(PICO_PIO_USB_DIR "${CMAKE_CURRENT_LIST_DIR}/external/Pico-PIO-USB")
set(PICO_PIO_USB_SRC_DIR "${PICO_PIO_USB_DIR}/src")

add_library(pico_pio_usb INTERFACE)
target_sources(pico_pio_usb INTERFACE
    ${PICO_PIO_USB_SRC_DIR}/pio_usb.c
    ${PICO_PIO_USB_SRC_DIR}/pio_usb_device.c
    ${PICO_PIO_USB_SRC_DIR}/pio_usb_host.c
    ${PICO_PIO_USB_SRC_DIR}/usb_crc.c
)
target_include_directories(pico_pio_usb INTERFACE ${PICO_PIO_USB_SRC_DIR})
target_link_libraries(pico_pio_usb INTERFACE
    pico_stdlib
    pico_multicore
    hardware_pio
    hardware_dma
)

add_executable(wup_1028_oc)

target_sources(wup_1028_oc PRIVATE
    src/main.c
    src/usb_descriptors.c
    src/wup_host.c
    ${PICO_TINYUSB_PATH}/src/portable/raspberrypi/pio_usb/hcd_pio_usb.c
)

target_include_directories(wup_1028_oc PRIVATE
    include
    src
)

target_compile_definitions(wup_1028_oc PRIVATE
    PIO_USB_USE_TINYUSB
)

target_compile_options(wup_1028_oc PRIVATE
    -Wall
    -Wextra
)

target_link_options(wup_1028_oc PRIVATE
    -Wl,--print-memory-usage
)

target_link_libraries(wup_1028_oc PRIVATE
    pico_stdlib
    pico_multicore
    pico_sync
    pico_pio_usb
    tinyusb_device
    tinyusb_host
)

pico_enable_stdio_usb(wup_1028_oc 0)
pico_enable_stdio_uart(wup_1028_oc 0)

add_custom_command(TARGET wup_1028_oc POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:wup_1028_oc>
            ${CMAKE_CURRENT_BINARY_DIR}/wup_1028_oc.bin
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/tools/bin_to_uf2.py
            ${CMAKE_CURRENT_BINARY_DIR}/wup_1028_oc.bin
            ${CMAKE_CURRENT_BINARY_DIR}/wup_1028_oc.uf2
    COMMENT "Generating wup_1028_oc.bin and wup_1028_oc.uf2"
)

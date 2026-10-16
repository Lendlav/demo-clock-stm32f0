cmake_minimum_required(VERSION 3.16)

project(demo_clock_stm32f0 LANGUAGES C ASM)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(STM32F0_HAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F0xx_HAL_Driver")
set(CMSIS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS")
set(PLATFORM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Platform/STM32F0")
set(FREERTOS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/FreeRTOS/Source")

set(FIRMWARE_SOURCES
    Application/clock_application.c
    ThirdParty/FreeRTOS/Source/tasks.c
    ThirdParty/FreeRTOS/Source/list.c
    ThirdParty/FreeRTOS/Source/queue.c
    ThirdParty/FreeRTOS/Source/portable/MemMang/heap_2.c
    ThirdParty/FreeRTOS/Source/portable/GCC/ARM_CM0/port.c
    Platform/STM32F0/main.c
    Platform/STM32F0/rtos_tasks.c
    Platform/STM32F0/gpio.c
    Platform/STM32F0/i2c.c
    Platform/STM32F0/rtc.c
    Platform/STM32F0/rtc_adapter.c
    Platform/STM32F0/clock_display.c
    Platform/STM32F0/stm32f0xx_hal_msp.c
    Platform/STM32F0/stm32f0xx_it.c
    Platform/STM32F0/system_stm32f0xx.c
    Platform/STM32F0/Display/ssd1306.c
    Platform/STM32F0/Startup/startup_stm32f030x6.S
    Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal.c
    Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_cortex.c
    Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_dma.c
    Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_flash.c
    Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_flash_ex.c
    Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_gpio.c
    Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_i2c.c
    Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_i2c_ex.c
    Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pwr.c
    Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pwr_ex.c
    Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rcc.c
    Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rcc_ex.c
    Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rtc.c
    Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rtc_ex.c
)

add_executable(demo-clock.elf ${FIRMWARE_SOURCES})

target_include_directories(demo-clock.elf PRIVATE
    Application
    Domain
    "${PLATFORM_DIR}"
    "${PLATFORM_DIR}/Inc"
    "${PLATFORM_DIR}/Display"
    "${FREERTOS_DIR}/include"
    "${FREERTOS_DIR}/portable/GCC/ARM_CM0"
    "${STM32F0_HAL_DIR}/Inc"
    "${STM32F0_HAL_DIR}/Inc/Legacy"
    "${CMSIS_DIR}/Device/ST/STM32F0xx/Include"
    "${CMSIS_DIR}/Include"
)

target_compile_definitions(demo-clock.elf PRIVATE
    USE_HAL_DRIVER
    STM32F030x6
)

target_compile_options(demo-clock.elf PRIVATE
    -mcpu=cortex-m0
    -mthumb
    -ffunction-sections
    -fdata-sections
    -fno-common
    -Wall
    -Wextra
    $<$<COMPILE_LANGUAGE:C>:-Os>
    $<$<COMPILE_LANGUAGE:C>:-g3>
)

set(LINKER_SCRIPT "${PLATFORM_DIR}/Linker/stm32f030f4.ld")
target_link_options(demo-clock.elf PRIVATE
    -mcpu=cortex-m0
    -mthumb
    -nostartfiles
    --specs=nano.specs
    --specs=nosys.specs
    "-T${LINKER_SCRIPT}"
    -Wl,--gc-sections
    -Wl,--print-memory-usage
    "-Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/demo-clock.map"
)
set_property(TARGET demo-clock.elf APPEND PROPERTY LINK_DEPENDS "${LINKER_SCRIPT}")

find_program(ARM_NONE_EABI_OBJCOPY NAMES arm-none-eabi-objcopy)
if(NOT ARM_NONE_EABI_OBJCOPY)
    message(FATAL_ERROR "arm-none-eabi-objcopy was not found in PATH")
endif()

add_custom_command(TARGET demo-clock.elf POST_BUILD
    COMMAND "${ARM_NONE_EABI_OBJCOPY}" -O ihex
            "$<TARGET_FILE:demo-clock.elf>"
            "${CMAKE_CURRENT_BINARY_DIR}/demo-clock.hex"
    COMMAND "${ARM_NONE_EABI_OBJCOPY}" -O binary
            "$<TARGET_FILE:demo-clock.elf>"
            "${CMAKE_CURRENT_BINARY_DIR}/demo-clock.bin"
    COMMENT "Generating Intel HEX and binary images"
    VERBATIM
)

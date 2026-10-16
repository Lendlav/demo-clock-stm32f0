# STM32F030 clock and ticker

Firmware for an STM32F030F4Px board with a 128x32 SSD1306 OLED connected to I2C1. The RTC supplies the displayed time and date; the ticker and battery gauge are demo animations.

Original demo video: <https://youtu.be/x5GeTt9Zqug>

## Architecture

- `Domain/` contains the clock data model and has no hardware or RTOS dependencies.
- `Application/` formats the clock view and owns the separator, ticker, and battery animation state.
- `Platform/STM32F0/` initializes the MCU, adapts the RTC and SSD1306 display, and coordinates FreeRTOS tasks.
- `Drivers/` contains the STM32 HAL and CMSIS device support.
- `ThirdParty/FreeRTOS/` contains the FreeRTOS kernel sources and GCC Cortex-M0 port.

The platform starts a clock-update task and a display task. They exchange rendered clock views through a one-element FreeRTOS queue; the domain and application layers do not depend on FreeRTOS. SysTick advances both the HAL and RTOS clocks.

## Build

Install CMake 3.16 or newer and the GNU Arm Embedded toolchain (`arm-none-eabi-gcc` and `arm-none-eabi-objcopy`), then run:

```sh
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/arm-none-eabi.cmake" \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The build writes `demo-clock.elf`, `demo-clock.hex`, `demo-clock.bin`, and `demo-clock.map` to `build/`. The linker layout targets the 16 KiB Flash and 4 KiB RAM configuration selected in the original project.

## Hardware connections

- OLED SCL: PA9 / I2C1_SCL
- OLED SDA: PA10 / I2C1_SDA
- The RTC uses the internal low-speed oscillator (LSI).

The RTC calendar is read at startup; firmware does not set an initial date or time.

The project uses the FreeRTOS V9.0.0 kernel retained from the original project, with the GNU Arm Cortex-M0 port. The Cortex-M0 port source identifies FreeRTOS Kernel V10.2.1; the port API remains compatible with this kernel configuration.

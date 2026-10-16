# STM32F030 Clock & Ticker

Firmware for an **STM32F030F4Px** microcontroller with a **128×32 SSD1306 OLED**. It displays the RTC date and time alongside a scrolling ticker and an animated battery gauge demo.

[▶ Watch the original demo](https://youtu.be/x5GeTt9Zqug)

The project uses CMake, the STM32 HAL, CMSIS, and FreeRTOS. Its application logic is separated from MCU-specific code so the clock formatting and animation logic do not depend on the hardware or RTOS.

## What it does

- Reads date and time from the STM32 RTC.
- Renders the clock on a 128×32 SSD1306 display over I²C.
- Shows a scrolling ticker and battery gauge animation.
- Runs clock updates and display refresh in separate FreeRTOS tasks.

> The RTC uses the internal low-speed oscillator (LSI). The firmware reads the RTC calendar but does not set its initial date or time.

## Project structure

| Directory | Contents |
| --- | --- |
| `Domain/` | Clock and date data types, independent of hardware and RTOS. |
| `Application/` | Clock view formatting and ticker/battery animation state. |
| `Platform/STM32F0/` | Startup, HAL configuration, RTC/display adapters, and FreeRTOS tasks. |
| `Drivers/` | STM32 HAL and CMSIS device support. |
| `ThirdParty/FreeRTOS/` | FreeRTOS kernel and GCC Cortex-M0 port. |
| `cmake/` | GNU Arm toolchain configuration for CMake. |

The clock task sends a rendered view through a one-item FreeRTOS queue; the display task draws it. SysTick provides the HAL and RTOS tick.

The ticker and battery gauge are demo animations; the gauge does not measure a real battery.

## Hardware

### OLED display

| OLED | STM32F030 |
| --- | --- |
| SCL | PA9 / I2C1_SCL |
| SDA | PA10 / I2C1_SDA |
| VCC, GND | Connect according to the display module's specifications; use 3.3 V logic. |

The firmware is configured for an SSD1306 128×32 display. Make sure the module's I²C pull-ups are suitable for 3.3 V.

### ST-Link SWD

| ST-Link | Target board |
| --- | --- |
| SWDIO | SWDIO |
| SWCLK | SWCLK |
| GND | GND |
| VTref / 3.3 V sense | Target 3.3 V reference |

Power the target board and connect a common ground. VTref is a voltage reference input on many ST-Link adapters; check your adapter before using any pin marked `3.3 V` as a power output.

## Prepare the build and flash tools

On Debian or Ubuntu, install CMake, the GNU Arm Embedded toolchain, Make, and ST-Link utilities:

```sh
sudo apt install cmake build-essential gcc-arm-none-eabi binutils-arm-none-eabi stlink-tools
```

Check that the commands are available:

```sh
cmake --version
arm-none-eabi-gcc --version
arm-none-eabi-objcopy --version
st-flash --version
```

Package names can differ on other Linux distributions. Install equivalent CMake, GNU Arm Embedded, Make, and ST-Link packages there.

## Build

From the repository root, configure and build the firmware:

```sh
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/arm-none-eabi.cmake" \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

The generated files are placed in `build/`:

| File | Use |
| --- | --- |
| `demo-clock.elf` | Debugging and symbol information. |
| `demo-clock.hex` | Intel HEX firmware image. |
| `demo-clock.bin` | Raw binary image used by the ST-Link command below. |
| `demo-clock.map` | Linker memory and symbol map. |

The linker configuration targets the STM32F030F4 memory layout: **16 KiB Flash** and **4 KiB RAM**.

## Flash with ST-Link

1. Connect the ST-Link to the target's SWDIO, SWCLK, GND, and target-voltage reference pins. Power the board.
2. Confirm that ST-Link can see the programmer and MCU:

   ```sh
   st-info --probe
   ```

3. Build the firmware if you have not already, then program the binary and reset the MCU:

   ```sh
   st-flash --reset write build/demo-clock.bin 0x08000000
   ```

`st-flash` writes the binary at the start of the STM32 internal Flash. The `--reset` option resets the target after programming, so the application starts immediately.

If the probe cannot connect while the application is running, retry with the target held in reset during connection:

```sh
st-flash --connect-under-reset --reset write build/demo-clock.bin 0x08000000
```

### Troubleshooting

- **No programmer or target found:** check the USB cable, target power, common GND, SWDIO/SWCLK wiring, and ST-Link permissions. Try `--connect-under-reset` if normal SWD connection fails.
- **OLED stays blank:** verify 3.3 V power, ground, SDA on PA10, SCL on PA9, and that the module is an SSD1306 128×32 I²C display.
- **Date or time is unexpected:** the firmware does not initialize the RTC calendar. Set it with a separate RTC setup/debug procedure.

## License notices

The bundled FreeRTOS V9.0.0 kernel is distributed under GPLv2 with the FreeRTOS exception; see [`ThirdParty/FreeRTOS/LICENSE.txt`](ThirdParty/FreeRTOS/LICENSE.txt). The GCC Cortex-M0 port has a separate MIT license notice in its source. Bundled CMSIS and STM32 HAL/device-support files retain their original notices in their source headers. Include applicable third-party notices when redistributing firmware binaries. No project-wide license is declared for the project's own code.

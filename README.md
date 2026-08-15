# stm32f407-drivers
Lightweight, register-level peripheral drivers for the STM32F407 (ARM Cortex-M4) microcontroller,
implemented in bare-metal C without vendor HAL libraries.

## Overview
This project implements a register-level abstraction of selected STM32F407 peripherals,
starting with GPIO and external interrupt (EXTI) support.

The current implementation supports:
- GPIO pin configuration (mode, output type, speed, pull-up/pull-down)
- SPI peripheral configuration, send, receive, and interrupt handling
- Digital read/write operations
- EXTI-based interrupt configuration and handling
- Peripheral clock control via RCC

### Technical Highlights
- Register-level peripheral control (no STM32 HAL)
- Safe interrupt handling using write-1-to-clear patterns
- Modular driver architecture
- Defensive parameter validation and status-based APIs
- Clear Doxygen-style documentation

## Demos
### GPIO
- **`001LED_toggle_PP`**: Demonstrates GPIO LED toggle capabilities using push/pull output.
- **`002LED_toggle_OD`**: Demonstrates GPIO LED toggle capabilities using open-drain output.
- **`003LED_button`**: Demonstrates GPIO input and output by having a hardware button power an on-board LED using GPIO.
- **`004button_interrupt`**: Demonstrates basic GPIO interrupt delivery functionality by having an on-board button deliver an interrupt. The ISR is then responsible for toggling the LED.
### SPI
- **`005SPI_TX`**: A basic demo demonstrating SPI TX capabilities.
- [**`006SPI_arduino_TX`**](./demos/006SPI_arduino_TX_README.md): This demo sends a length byte followed by a payload string to an Arduino SPI slave from a STM32F407 master.

## Directory Structure
```
stm32f4xx_drivers/
├── Inc/
├── Src/
│   ├── 001LED_toggle_PP.c              # LED blink demo using push/pull GPIO output
│   ├── 002LED_toggle_OD.c              # LED blink demo using open-drain GPIO output
│   ├── 003LED_button.c                 # LED toggle demo using on-board button via GPIO.
│   ├── 004button_interrupt.c           # GPIO interrupt delivery demo using button & LED.
│   ├── 005SPI_TX.c                     # Basic SPI TX demo.
│   ├── 006SPI_arduino_TX.c             # SPI TX demo with Arduino Slave.
│   ├── syscalls.c                      # Stm32CubeIDE auto-generated stub system calls.
│   └── sysmem.c                        # Stm32CubeIDE auto-generated heap management lib.
├── Startup/
│   └── startup_stm32f407vgtx.s         # Startup assembly file: vector table, reset handler, etc.
├── arduino/
│   └── 006SPI_arduino_RX.ino           # Ardunio SPI slave to test SPI TX functionality.
├── demos/ 
│   └── 006SPI_arduino_TX_README.md     # Documentation for SPI Arduino slave demo.
├── docs/
│   ├── images/                         # Images of Logic Analyzer Output or Hardware Config.
│   └── function_docs_template.txt      # Doxygen template
├── drivers/
│   ├── Inc/
│   │   ├── stm32f407xx.h               # Device-specific header
│   │   ├── stm32f407xx_gpio_driver.h   # GPIO-driver header file
│   │   └── stm32f407xx_spi_driver.h    # SPI-driver header file
│   ├── Src/
│   │   ├── stm32f407xx_gpio_driver.c   # GPIO-driver C file
│   │   └── stm32f407xx_spi_driver.c    # SPI-driver C file
├── STM32F407VGTX_FLASH.ld              # Linker script for running firmware from flash.
├── STM32F407VGTX_RAM.ld                # Linker script for running firmware from SRAM.
└── README.md
```

## Driver Architecture
The drivers are structured as:
- Device Header File (`stm32f407xx.h`): Contains important MCU address macros and register definitions.
- Peripheral Driver Interface (_`drivers/*.h` files_): Public API made available to user applications.
- Peripheral Driver Implementation (_`drivers/*.c` files_): Register-level logic which implements the API functions.

## Toolchain
- STM32CubeIDE
- ARM GCC Toolchain
- STM32F407G Discovery Board (ARM Cortex-M4, 168MHz)
- Bare-Metal (_no RTOS_)

## Roadmap
Planned driver implementations:
- [X] GPIO
- [X] SPI
- [ ] I2C
- [ ] UART/USART
- [ ] Timer Peripherals
- [ ] DMA Integration

## Future Improvements
- [ ] Unit-Testing Framework
- [ ] Low-Power Mode Handling
- [ ] Support for FreeRTOS Integration

# stm32f407-drivers
Lightweight, register-level peripheral drivers for the STM32F407 (ARM Cortex-M4) microcontroller,
implemented in bare-metal C without vendor HAL libraries.

## Overview
This project implements a register-level abstraction of selected STM32F407 peripherals, including GPIO,
SPI, I2C, NVIC, and RCC drivers.

The current implementation supports:
- GPIO pin configuration (mode, output type, speed, pull-up/pull-down)
- Digital read/write operations
- SPI peripheral configuration, send, receive, and interrupt handling
- I2C peripheral configuration, master/slave transfers, and interrupt handling
- NVIC interrupt enabling/disabling, and interrupt priority configuration
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
- [**`006SPI_arduino_TX`**](./demos/006SPI_arduino_TX_README.md): This demo sends a length byte followed by a payload string to an Arduino SPI slave from an STM32F407 master.
- [**`007SPI_arduino_CMD`**](./demos/007SPI_arduino_CMD_README.md): This demo makes use of full-duplex SPI communication using the custom SPI driver to send commands to an Arduino Uno slave from an STM32F407 master.
- [**`008SPI_arduino_interrupt`**](./demos/008SPI_arduino_interrupt.md): This demo makes use of the interrupt-driven SPI capabilities of the driver by having the STM32 SPI-master receive a variable-length message from an Arduino Uno.
### I2C
- [**`009I2C_MasterTx`**](./demos/009I2C_MasterTx_README.md): This demo uses the custom I2C drivers on an STM32F407 Discovery board to transmit data to an Arduino Uno configured as an I2C slave.
- [**`010I2C_MasterRx`**](./demos/010I2C_MasterRx_README.md): This demo uses the custom I2C drivers on an STM32F407 Discovery board acting as an I2C master to receive length and data information from an Arduino Uno configured as an I2C slave.
- [**`011I2C_MasterRxInterrupt`**](./demos/011I2C_MasterRxInterrupt_README.md): This demo uses the custom interrupt-driven I2C drivers on an STM32F407 Discovery board acting as an I2C master to receive length and data information from an Arduino Uno configured as an I2C slave.
- [**`012I2C_SlaveTxInterrupt`**](./demos/012I2C_SlaveTxInterrupt_README.md): This demo uses the custom interrupt-driven I2C-slave code for the STM32F407 to send a variable-length message to an Arduino acting as an I2C master.
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
│   ├── 007SPI_arduino_CMD.c            # SPI full-duplex demo with Arduino Slave.
│   ├── 008SPI_arduino_interrupt.c      # SPI interrupt-driven demo with Arduino Slave.
│   ├── 009I2C_MasterTx.c               # I2C-master transmit demo.
│   ├── 010I2C_MasterRx.c               # I2C-master receive demo.
│   ├── 011I2C_MasterRxInterrupt.c      # Interrupt-driven I2C-master receive demo.
│   ├── 012I2C_SlaveTxInterrupt.c       # Interrupt-driven I2C-slave transmit demo.
│   ├── syscalls.c                      # STM32CubeIDE auto-generated stub system calls.
│   └── sysmem.c                        # STM32CubeIDE auto-generated heap management lib.
├── Startup/
│   └── startup_stm32f407vgtx.s         # Startup assembly file: vector table, reset handler, etc.
├── arduino/
│   ├── 006SPI_arduino_RX.ino           # Arduino SPI slave to test SPI TX functionality.
│   ├── 007_SPI_arduino_CMD.ino         # Arduino SPI slave code to test full-duplex functionality.
│   └── 008_SPI_arduino_interrupt.ino   # Arduino SPI slave code to test SPI interrupt-support.
├── demos/                              # Demo documentation
├── docs/
│   ├── images/                         # Images of Logic Analyzer Output or Hardware Config.
│   └── function_docs_template.txt      # Doxygen template
├── drivers/
│   ├── Inc/
│   │   ├── stm32f407xx.h               # Device-specific header
│   │   ├── stm32f407xx_gpio_driver.h   # GPIO-driver header file
│   │   ├── stm32f407xx_spi_driver.h    # SPI-driver header file
│   │   ├── stm32f407xx_nvic_driver.h   # NVIC-driver header file
│   │   ├── stm32f407xx_rcc_driver.h    # RCC-driver header file
│   │   └── stm32f407xx_i2c_driver.h    # I2C-driver header file
│   └── Src/
│       ├── stm32f407xx_gpio_driver.c   # GPIO-driver C file
│       ├── stm32f407xx_spi_driver.c    # SPI-driver C file
│       ├── stm32f407xx_nvic_driver.c   # NVIC-driver C file
│       ├── stm32f407xx_rcc_driver.c    # RCC-driver C file
│       └── stm32f407xx_i2c_driver.c    # I2C-driver C file
├── STM32F407VGTX_FLASH.ld              # Linker script for running firmware from flash.
├── STM32F407VGTX_RAM.ld                # Linker script for running firmware from SRAM.
└── README.md
```

## Driver Architecture
The drivers are structured as:
- Device Header File (`drivers/Inc/stm32f407xx.h`): Contains important MCU address macros and register definitions.
- Peripheral Driver Interface (`drivers/Inc/*.h` files): Public API made available to user applications.
- Peripheral Driver Implementation (`drivers/Src/*.c` files): Register-level logic which implements the API functions.

## Toolchain
- STM32CubeIDE
- ARM GCC Toolchain
- STM32F407G Discovery Board (ARM Cortex-M4, 168MHz)
- Bare-Metal (_no RTOS_)

## Roadmap
Planned driver implementations:
- [X] GPIO
- [X] NVIC
- [X] RCC
- [X] SPI
- [X] I2C
- [ ] UART/USART
- [ ] Timer Peripherals
- [ ] DMA Integration

## Future Improvements
- [ ] Unit-Testing Framework
- [ ] Low-Power Mode Handling
- [ ] Support for FreeRTOS Integration

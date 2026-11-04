# 010 I2C Master Receive

### Overview
This demo uses the custom I2C drivers on an STM32F407 Discovery board acting as an I2C master to receive length and data information from an Arduino Uno configured as an I2C slave.

The STM32 controller waits for the user-button to be pressed. Once done, acting as an I2C master, it sends a command to the Arduino to request the length of the message in bytes. It then issues a repeating start to read the length information from the Arduino. Next, the STM32 master sends another command using a repeated start to request the message itself from the Arduino. After another repeated start, the Arduino transmits the message to the STM32. The STM32 NACKs the last byte of the message and issues a stop-condition, which frees the bus.

Note that the use of repeated starts enables the STM32 to hold the bus until all transfers are finished, preventing other masters from acquiring ownership of the bus.

The Arduino slave uses the `<Wire.h>` library to read commands and send data using I2C.

## Files
- **STM32 Master File**: `Src/010I2C_MasterRx.c`
- **Arduino Slave File**: `arduino/010_I2C_SlaveTxString/010_I2C_SlaveTxString.ino`

## Wiring
The hardware required to replicate this demo includes:
- Arduino Uno Board
- STM32F407G Discovery Board
- Breadboard
- 3.3V/5V Bi-directional, I2C-compatible Level Shifter
- Jumper Wires
- Pull-up Resistors
- Logic Analyzer (*optional*)

1. Connect the level-shifter to 3.3V (*from the STM32 board*) and 5V (*from the Arduino*), and a common ground (*from the Arduino **and** STM32 board*). Note that the **LV** (*low-voltage*) side of the level-shifter should be connected to 3.3V, and the **HV** (*high-voltage*) side should be connected to 5V.
2. `A5 -> PB6`: Connect the **SCL** of the Arduino (Pin A5) to the **SCL** of I2C1 on the STM32 (PB6) through the level shifter.
3. `A4 -> PB7`: Connect the **SDA** of the Arduino (Pin A4) to the **SDA** of I2C1 on the STM32 (PB7) through the level shifter.
4. If the level-shifter does not contain built-in pull-ups, connect external pull-ups to each side of the level-shifter, on both the SCL and SDA lines.
5. (*optional*) Connect the logic analyzer to the breadboard rows to snoop data. Note that this will increase the capacitance of the I2C bus.

## Images

### Logic Analyzer Sample
![Logic Analyzer](../docs/images/010_I2C_MasterRx_logic_analyzer.png)
- *The decoded data can be seen in the bottom-right panel.*

### Arduino Serial Terminal Output
![Arduino Serial Terminal Output](../docs/images/010_I2C_MasterRx_serial_monitor.png)

### STM32F407 SWV Output
![Master Serial-Wire View Output](../docs/images/010I2C_MasterRx_SWV.png)
- *The SWV output can be seen in the bottom-middle panel.*

### Hardware Setup
![Hardware Setup](../docs/images/009I2C_MasterTx_hardware.jpeg)

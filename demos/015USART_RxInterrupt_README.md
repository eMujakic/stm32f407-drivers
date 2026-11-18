# 015 USART Interrupt-Driven Rx Demo

### Overview
TODO

## Files
- **STM32 File**: `Src/015USART_RxInterrupt.c`
- **Arduino File**: `arduino/014_USART_Arduino_Tx/014_USART_Arduino_Tx.ino`

## Wiring
The hardware required to replicate this demo includes:
- Arduino Uno Board
- STM32F407G Discovery Board
- Breadboard
- 3.3V/5V Bi-directional Level Shifter
- Jumper Wires
- Pull-up Resistors
- Logic Analyzer (*optional*)

1. Connect the level-shifter to 3.3V (*from the STM32 board*) and 5V (*from the Arduino*), and a common ground (*from the Arduino **and** STM32 board*). Note that the **LV** (*low-voltage*) side of the level-shifter should be connected to 3.3V, and the **HV** (*high-voltage*) side should be connected to 5V.
2. `D0 -> PA2`: Connect the **RX** of the Arduino (Pin D0) to the **TX** of USART2 on the STM32 (PA2) through the level shifter.
3. `D1 -> PA3`: Connect the **TX** of the Arduino (Pin D1) to the **RX** of USART2 on the STM32 (PA3) through the level shifter.
4. If the level-shifter does not contain built-in pull-ups, connect external pull-ups to each side of the level-shifter, on both the TX and RX lines.
5. (*optional*) Connect the logic analyzer to the breadboard rows to snoop data.

## Images

### Logic Analyzer Sample
![Logic Analyzer](../TODO)
- *The decoded data can be seen in the bottom-right panel.*

### STM32 Serial Wire Viewer Output
![STM32 Serial Wire Viewer Output](../TODO)

### Hardware Setup
![Hardware Setup](../TODO)

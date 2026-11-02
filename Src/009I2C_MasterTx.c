 /***************************************************************************
 * @file		009I2C_MasterTx.c
 * @author		ernad
 * @date 		Nov 1, 2026
 * @brief		I2C demo to send data to an Arduino slave from STM32F407.
 *
 * @details
 * This demo uses the custom I2C drivers on an STM32F407 Discovery board to 
 * transmit data to an Arduino Uno configured as an I2C slave.
 * 
 * A bidirectional I2C-compatible level shifter connects the STM32's 3.3V
 * pins to the Arduino's 5V pins. Both boards and the level shifter must
 * share a common ground.
 *
 * This demo disables internal pull-up resistors, so external pull-ups
 * must be present on both sides of the level shifter. If the level-
 * shifter contains built-in pull-ups, additional resistors are 
 * usually unnecessary.
 * 
 * Connections through the level shifter:
 * SCL: Stm32-B6 <-> Arduino-A5
 * SDA: Stm32-B7 <-> Arduino-A4
 *
 * Arduino Slave-Address: 0x68
 *
 * @note    This setup was tested using 10K ohm pull-up resistors built
 *          into the level shifter. Required pull-up values depend on 
 *          total bus capacitance. Keep breadboard connections and wires
 *          as short as possible.
 ***************************************************************************/

#include <stdint.h>
#include "stm32f407xx.h"
#include <string.h>
#include <stdio.h>

#define I2C1_AF_MODE    GPIO_AF4
#define I2C1_PORT       GPIOB

#define I2C1_SCL_PIN    GPIO_PIN_6  
#define I2C1_SDA_PIN    GPIO_PIN_7

// GPIO port/pin of hardware button on STM32f407G Disc. Board
#define BUTTON_PORT		GPIOA
#define BUTTON_PIN		GPIO_PIN_0

#define ARDUINO_ADDR    0x68

// ====================== GLOBAL VARIABLES ======================

// Global I2C1 handle
I2C_handle_t i2c1;

// Global Tx-buffer
uint8_t TxBuf[] = "Hello from STM32!"; // NOTE: limit message to 32-bytes due to Arduino library limitations

// ====================== END-GLOBAL VARIABLES ======================

// Initializes GPIO pins for I2C1 peripheral
static void gpio_init()
{
    GPIO_handle_t gpio_i2c1;
    GPIO_ConfigInit(&gpio_i2c1.GPIO_PinConfig);

    gpio_i2c1.pGPIOx = I2C1_PORT;
    gpio_i2c1.GPIO_PinConfig.mode           = GPIO_MODE_ALTFN;  // alternate-functionality mode
	gpio_i2c1.GPIO_PinConfig.outType        = GPIO_OTYPE_OD;    // open-drain
	gpio_i2c1.GPIO_PinConfig.puPdCtl        = GPIO_NO_PUPD;     // no internal pull-up/down
	gpio_i2c1.GPIO_PinConfig.speed          = GPIO_SPEED_HIGH;
    gpio_i2c1.GPIO_PinConfig.altFuncMode    = I2C1_AF_MODE;     // set AF mode to I2C1

    // SCL
	gpio_i2c1.GPIO_PinConfig.pinNumber      = I2C1_SCL_PIN;
    GPIO_Init(&gpio_i2c1);

    // SDA
    gpio_i2c1.GPIO_PinConfig.pinNumber      = I2C1_SDA_PIN;
    GPIO_Init(&gpio_i2c1);
}

// Initialize I2C1 peripheral
static void i2c1_init()
{
    i2c1.pI2Cx = I2C1;
    I2C_ConfigInit(&i2c1.I2C_Config); // initialize config struct to safe defaults

    status_t status = I2C_Init(&i2c1);
    printf("I2C_Init Status: %d\n", status);
}

// Initializes GPIO pin for physical button
static void gpio_btn_init()
{
    GPIO_handle_t gpio_button;
	gpio_button.pGPIOx                      = BUTTON_PORT;
	gpio_button.GPIO_PinConfig.pinNumber    = BUTTON_PIN;
	gpio_button.GPIO_PinConfig.mode         = GPIO_MODE_IN;
	gpio_button.GPIO_PinConfig.puPdCtl      = GPIO_NO_PUPD;

	GPIO_Init(&gpio_button);
}

// basic busy-waiting helper function
static void delay(uint32_t val)
{
	for(uint32_t i = 0; i < val; i++);
}


// helper function which waits for button press and debounces
static void wait_for_press(void)
{
    while( !GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN) );    // wait for button press
    delay(5000);                                        // debouncing

    if( !GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN) )        // check if still pressed
        return;

    while( GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN) );     // wait for button release

    delay(5000);                                        // debouncing
    return;
}

int main(void)
{
    gpio_btn_init();
    gpio_init();
    i2c1_init();

    // disable printf buffering
    setvbuf(stdout, NULL, _IONBF, 0);

    // send data to Arduino
    status_t status = STATUS_OK;
    while(1)
    {
        wait_for_press();
        status = I2C_MasterSendData(&i2c1, TxBuf, sizeof(TxBuf)-1U, ARDUINO_ADDR);
        printf("MasterSendData Status: %d", status);
    }

    return 0;
}

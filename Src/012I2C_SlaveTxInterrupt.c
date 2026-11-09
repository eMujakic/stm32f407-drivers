/***************************************************************************
 * @file		012I2C_SlaveTxInterrupt.c
 * @author		ernad
 * @date 		Nov 9, 2026
 * @brief		I2C application to demonstrate functionality and usage of
 *              custom interrupt-driven I2C slave driver APIs.
 *
 * @details
 * This demo uses the custom I2C interrupt drivers on an STM32F407
 * Discovery board acting as an I2C slave to send length and data
 * information to an Arduino Uno configured as an I2C master.
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
 * @note    This setup was tested using 10K ohm pull-up resistors built
 *          into the level shifter. Required pull-up values depend on
 *          total bus capacitance. Keep breadboard connections and wires
 *          as short as possible.
 *
 * @note    This demo makes use of repeated starts.
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

#define MY_ADDR         0x68

// Command codes
#define GET_LEN_CMD     0x51    // Request message length
#define GET_DATA_CMD    0x52    // Request message data

// ====================== GLOBAL VARIABLES ======================

// Global I2C1 handle
I2C_handle_t i2c1;
uint8_t TxBuffer[32] = "Hello from STM32!";
uint8_t message_length = 0U;
uint8_t cmd = 0U;

// global flags
volatile uint8_t failure = DISABLE;

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

    i2c1.I2C_Config.I2C_DeviceAddr = MY_ADDR;

    status_t status = I2C_Init(&i2c1);
    printf("I2C_Init Status: %d\n", status);
}

// helper function to reset i2c peripheral and failure flag
static void reset_i2c()
{
    I2C_SoftwareReset(i2c1.pI2Cx);
    I2C_Init(&i2c1);
    failure = 0U;

    // restart listening
    I2C_SlaveListenIT(&i2c1);
}

int main(void)
{  
    gpio_init();
    i2c1_init();

    // disable printf buffering
    setvbuf(stdout, NULL, _IONBF, 0);

    // initialize message length variable
    message_length = (uint8_t)strlen(TxBuffer);

    // i2c IRQ configs
    NVIC_EnableIRQ(IRQ_NUM_I2C1_ER); // enable error interrupts
    NVIC_EnableIRQ(IRQ_NUM_I2C1_EV); // enable event interrupts

    // enable slave listening
    I2C_SlaveListenIT(&i2c1);

    while(1)
    {
        if (failure) reset_i2c();
    }
    return 0;
}

// Event handler
void I2C1_EV_IRQHandler(void)
{
    I2C_IRQHandling_EV(&i2c1);
}

// Error handler
void I2C1_ER_IRQHandler(void)
{
    I2C_IRQHandling_ERR(&i2c1);
}

void I2C_ApplicationEventCallback(I2C_handle_t *pHandle, I2C_event_t event)
{
    if (event == I2C_EVT_SLAVE_ADDR_MATCH)
    {
        if (pHandle->TxRxState == I2C_BUSY_IN_TX)
        {
            if (cmd == GET_LEN_CMD)
                I2C_SlaveSendDataIT(&i2c1, &message_length, 1U); // send message length
                
            else if (cmd == GET_DATA_CMD)
                I2C_SlaveSendDataIT(&i2c1, TxBuffer, message_length); // send message
        }
        else if (pHandle->TxRxState == I2C_BUSY_IN_RX)
        {
            I2C_SlaveReceiveDataIT(&i2c1, &cmd, 1U); // receive command code
        }
        else
        {
            failure = ENABLE;
        }
    }
    else if(event == I2C_EVT_RX_DONE ||
            event == I2C_EVT_TX_DONE || 
            event == I2C_EVT_STOP)
    {
        ;
    }
    else // some unexpected event
    {
        failure = ENABLE;
    }
}
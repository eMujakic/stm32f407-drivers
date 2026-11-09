 /***************************************************************************
 * @file		011I2C_MasterRxInterrupt.c
 * @author		ernad
 * @date 		Nov 7, 2026
 * @brief		I2C demo to receive n-bytes of data from an Arduino slave
 *              using custom interrupt-driven APIs.
 *
 * @details
 * This demo uses the custom I2C interrupt drivers on an STM32F407
 * Discovery board acting as an I2C master to receive length and data
 * information from an Arduino Uno configured as an I2C slave.
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
 *
 * @note    This demo allows the master to read at-most 256 bytes of data
 *          in one transfer.
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

#define MY_ADDR         0x61
#define ARDUINO_ADDR    0x68

// Command codes to send to Arduino
#define GET_LEN_CMD     0x51    // Request message length
#define GET_DATA_CMD    0x52    // Request message data

// ====================== GLOBAL VARIABLES ======================

// Global I2C1 handle
I2C_handle_t i2c1;

// global flags
volatile uint8_t Tx_Done = 0;
volatile uint8_t Rx_Done = 0;
volatile uint8_t failure = 0;

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

// helper function to reset i2c peripheral and failure flag
static void reset_i2c()
{
    I2C_SoftwareReset(i2c1.pI2Cx);
    I2C_Init(&i2c1);
    failure = 0U;
}

int main(void)
{
    gpio_btn_init();
    gpio_init();
    i2c1_init();

    // disable printf buffering
    setvbuf(stdout, NULL, _IONBF, 0);

    uint8_t buf[256]; // can read max 256 bytes of data at once

    // i2c IRQ configs
    NVIC_EnableIRQ(IRQ_NUM_I2C1_ER); // enable error interrupts
    NVIC_EnableIRQ(IRQ_NUM_I2C1_EV); // enable event interrupts

    uint8_t n = 0; // length of Arduino's message
    uint8_t cmd = GET_LEN_CMD;
    status_t status = STATUS_OK;
    while(1)
    {
        wait_for_press();

        // clear flags
        failure = 0;
        Tx_Done = 0;
        Rx_Done = 0;

        // clear buffer
        memset(buf, 0, sizeof(buf));

        // send REQUEST_LEN command code
        cmd = GET_LEN_CMD;
        Tx_Done = 0;
        status = I2C_MasterSendDataIT(&i2c1, &cmd, 1U, ARDUINO_ADDR, I2C_SR_EN); // repeated-start enabled
        printf("MasterRequestLen Status: %d\n", status);
        if (status != STATUS_OK)
        {
            reset_i2c();
            continue;
        }
            
        // read length
        Rx_Done = 0;
        while(!Tx_Done)
        {
            if( failure )               
                break;
        }
        if (failure)
        {
            reset_i2c();
            continue;
        }
        status = I2C_MasterReceiveDataIT(&i2c1, &n, 1U, ARDUINO_ADDR, I2C_SR_EN);
        printf("MasterReceiveLen Status: %d\n", status);
        if (status != STATUS_OK)
        {
            reset_i2c();
            continue;
        }
        

        // send REQUEST_DATA command code
        cmd = GET_DATA_CMD;
        Tx_Done = 0;
        while(!Rx_Done)
        {
            if( failure )
                break;
        }
        if (failure)
        {
            reset_i2c();
            continue;
        }
        status = I2C_MasterSendDataIT(&i2c1, &cmd, 1U, ARDUINO_ADDR, I2C_SR_EN);
        printf("MasterRequestData Status: %d\n", status);
        if (status != STATUS_OK)
        {
            reset_i2c();
            continue;
        }


        // read data
        Rx_Done = 0;
        while(!Tx_Done)
        {
            if( failure )               
                break;
        }
        if (failure)
        {
            reset_i2c();
            continue;
        }
        status = I2C_MasterReceiveDataIT(&i2c1, buf, n, ARDUINO_ADDR, I2C_SR_DI); // repeated-start disabled
        printf("MasterReceiveData Status: %d\n", status);
        if (status != STATUS_OK)
        {
            reset_i2c();
            continue;
        }

        while(!Rx_Done)
        {
            if( failure )               
                break;
        }
        if (failure)
        {
            reset_i2c();
            continue;
        }
        // print data
        printf("Message Received: %s\n", buf);
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
    if (event == I2C_EVT_TX_DONE)
    {
        Tx_Done = 1;
    }
    else if (event == I2C_EVT_RX_DONE)
    {
        Rx_Done = 1;
    }
    else
    {
        printf("Error Event Detected: %d\n", event);
        failure = 1;
    }
}

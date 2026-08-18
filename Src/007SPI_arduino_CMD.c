/*
 * @file		007SPI_arduino_CMD.c
 * @author		ernad
 * @date 		Aug 15, 2026
 * @brief		TODO
 *
 * @details
 * TODO
 */

#include <stdint.h>
#include "stm32f407xx.h"
#include <string.h>

#define SPI_ACK         0xF5U
#define SPI_NACK        0xA5U

#define SPI2_AF         GPIO_AF5    // SPI2 alternate-function mode
#define SPI2_PORT       GPIOB       // SPI2 GPIO port

#define SPI2_MOSI_PIN   GPIO_PIN_15   
#define SPI2_MISO_PIN   GPIO_PIN_14
#define SPI2_SCLK_PIN   GPIO_PIN_13
#define SPI2_NSS_PIN    GPIO_PIN_12

// GPIO port/pin of hardware button on STM32f407G Disc. Board
#define BUTTON_PORT		GPIOA
#define BUTTON_PIN		GPIO_PIN_0

// Command codes
#define CMD_LED_CTL         0x50
#define CMD_SENSOR_READ     0x51
#define CMD_LED_READ        0x52
#define CMD_PRINT           0x53
#define CMD_ID_READ         0x54

#define LED_OFF             0
#define LED_ON              1

#define LED_0               0
#define LED_1               1

// length of slave ID
#define ID_LEN              10

// basic busy-waiting helper function
static void delay(uint32_t val)
{
	for(uint32_t i = 0; i < val; i++);
}

// Initializes GPIO pins for SPI2 peripheral
static void gpio_init()
{
    GPIO_handle_t gpio_spi2;
    GPIO_ConfigInit(&gpio_spi2.GPIO_PinConfig);

    gpio_spi2.pGPIOx = SPI2_PORT;
    gpio_spi2.GPIO_PinConfig.mode           = GPIO_MODE_ALTFN;
	gpio_spi2.GPIO_PinConfig.outType        = GPIO_OTYPE_PP;
	gpio_spi2.GPIO_PinConfig.puPdCtl        = GPIO_NO_PUPD;
	gpio_spi2.GPIO_PinConfig.speed          = GPIO_SPEED_HIGH;
    gpio_spi2.GPIO_PinConfig.altFuncMode    = SPI2_AF;

    // MOSI
	gpio_spi2.GPIO_PinConfig.pinNumber      = SPI2_MOSI_PIN;
    GPIO_Init(&gpio_spi2);

    // MISO
    gpio_spi2.GPIO_PinConfig.pinNumber      = SPI2_MISO_PIN;
    GPIO_Init(&gpio_spi2);

    // SCLK
    gpio_spi2.GPIO_PinConfig.pinNumber      = SPI2_SCLK_PIN;
	GPIO_Init(&gpio_spi2);

    // NSS
    gpio_spi2.GPIO_PinConfig.pinNumber      = SPI2_NSS_PIN;
    GPIO_Init(&gpio_spi2);
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

// Initializes SPI2 peripheral
static void spi2_init(SPI_handle_t *spi2_handle)
{
    spi2_handle->pSPIx = SPI2;
    SPI_ConfigInit(&spi2_handle->SPI_Config);

    spi2_handle->SPI_Config.SPI_DeviceMode      = SPI_MODE_MASTER;
    spi2_handle->SPI_Config.SPI_BaudPrescaler   = SPI_BAUD_DIV8;       // 2MHz
    spi2_handle->SPI_Config.SPI_BusConfig       = SPI_BUS_FULL_DUPLEX;
    spi2_handle->SPI_Config.SPI_DFF 		    = SPI_DFF_8;
    spi2_handle->SPI_Config.SPI_CPOL            = SPI_CPOL_LOW;
    spi2_handle->SPI_Config.SPI_CPHA            = SPI_CPHA_FIRST_EDGE;
    spi2_handle->SPI_Config.SPI_SSM             = SPI_SSM_DI;

    SPI_Init(spi2_handle);
    SPI_PeripheralControl(SPI2, ENABLE);
}

// helper function which waits for button press and debounce
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

// sends command code and returns ack/nack from Arduino
uint8_t ack_or_nack (uint8_t cmd)
{
    uint8_t ack_byte = 0U;

    SPI_Send(SPI2, &cmd, 1);            // send command code
    SPI_Receive(SPI2, &ack_byte, 1);    // receive ack/nack byte

    return ack_byte;
}

// performs CMD_LED_CTL command
void perform_led_ctl (uint8_t pin, uint8_t val)
{
    wait_for_press();
    SPI_PeripheralControl(SPI2, ENABLE);

    if( ack_or_nack(CMD_LED_CTL) )
    {
        uint8_t args[] = { pin, val };
        SPI_Send(SPI2, args, 2);    // send args
    }
    SPI_PeripheralControl(SPI2, DISABLE);
}

// performs CMD_SENSOR_READ command
uint8_t perform_sensor_read (uint8_t pin)
{
    wait_for_press();
    SPI_PeripheralControl(SPI2, ENABLE);
    uint8_t val = 0;

    if( ack_or_nack(CMD_SENSOR_READ) )
    {
        SPI_Send(SPI2, &pin, 1);

        delay(500); // delay to ensure analog-read is ready

        SPI_Receive(SPI2, &val, 1);
    }
    SPI_PeripheralControl(SPI2, DISABLE);

    return val;
}

// performs CMD_LED_READ command
uint8_t perform_led_read (uint8_t led)
{
    wait_for_press();
    SPI_PeripheralControl(SPI2, ENABLE);
    uint8_t val = 0;

    if( ack_or_nack(CMD_LED_READ) )
    {
        SPI_Send(SPI2, &led, 1);

        delay(500); // delay to ensure digital-read is ready

        SPI_Receive(SPI2, &val, 1);
    }
    SPI_PeripheralControl(SPI2, DISABLE);

    return val;
}

// performs CMD_PRINT command
void perform_print(uint8_t *buf, uint8_t len)
{
    wait_for_press();
    SPI_PeripheralControl(SPI2, ENABLE);

    if ( ack_or_nack(CMD_PRINT) )
    {
        SPI_Send(SPI2, &len, 1);
        SPI_Send(SPI2, buf, len);
    }

    SPI_PeripheralControl(SPI2, DISABLE);
}

// performs CMD_ID_READ command
void perform_id_read(uint8_t *buf)
{
    wait_for_press();
    SPI_PeripheralControl(SPI2, ENABLE);

    if ( ack_or_nack(CMD_ID_READ) )
    {
        SPI_Receive(SPI2, buf, ID_LEN);
    }

    SPI_PeripheralControl(SPI2, DISABLE);
}

int main(void)
{
    gpio_init();        // initialzes GPIO pins for SPI2 AltFN modes
    gpio_btn_init();    // initialzes GPIO pin for hardware button input

    SPI_handle_t spi2_handle;
    spi2_init(&spi2_handle);
    SPI_SSOEConfig(SPI2, ENABLE);   // enables SS output

    while(1)
    {

        // =========== CMD_LED_CTRL(pinNum{0,1}, value{0,1}) ===========

        // turn on LED 0
        perform_led_ctl(0, 1);
        perform_led_read(0);

        // turn on LED 1
        perform_led_ctl(1, 1);

        // turn off LED 0
        perform_led_ctl(0, 0);
        perform_led_read(0);

        // turn off LED 1
        perform_led_ctl(1, 0);

        // =========== CMD_SENSOR_READ(pinNum{0-5}) ===========
        uint8_t sensor = perform_sensor_read(1);

        // =========== CMD_PRINT(buf, len) ===========
        char buf[] = "Hello from STM32!";
        perform_print((uint8_t*)buf, strlen(buf));

        // =========== CMD_ID_READ(buf) ===========
        char id[ID_LEN];
        perform_id_read((uint8_t*)id);
    }
}

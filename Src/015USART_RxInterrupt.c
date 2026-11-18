/***************************************************************************
 * @file		015USART_RxInterrupt.c
 * @author		ernad
 * @date 		Nov 18, 2026
 * @brief		This demo uses the custom interrupt-driven USART/UART
 *              drivers for the STM32F407 to receive a variable-length
 *              message from an Arduino Uno board.
 *
 * @details
 * TODO
 *
 * 1.   The STM32 sends the REQ_LEN_CMD (0x51) command code to the Arduino
 *      to request the length of the message.
 * 2.   The STM32 then reads the length of the message into the len
 *      variable.
 * 3.   The STM32 sends the SEND_MSG_CMD (0x52) to the Arduino, to request
 *      the message itself.
 * 4.   Finally, the STM32 reads the message into a buffer, and prints
 *      the buffer to the user.
 *
 * Baud rate, word-length, parity, stop-bits, oversampling, and
 * hardware flow-control use the defaults provided by USART_ConfigInit().
 * The Arduino peer must match these settings for successful
 * communication.
 *
 * TODO
 *
 * A bidirectional level shifter connects the STM32's 3.3V pins to the
 * Arduino's 5V pins. Both boards and the level shifter must
 * share a common ground.
 *
 * Connections through the level shifter:
 * Stm32-PA2 (TX) <-> Arduino-D0 (RX)
 * Stm32-PA3 (RX) <-> Arduino-D1 (TX)
 *
 * @note    The STM32's Rx-buffer can hold at most 256 bytes of data.
 *          The Arduino should not send more than 255 bytes to ensure
 *          room for the null terminator '\0'.
 *
 * @note    On Arduino boards that share the hardware UART with the USB
 *          interface (Uno R3), disconnect the STM32 Tx wire from Arduino
 *          Rx (D0) before uploading a sketch to avoid interference with
 *          the bootloader. Reconnect it after upload.
 ***************************************************************************/

#include <stdint.h>
#include "stm32f407xx.h"
#include <string.h>
#include <stdio.h>

#define USART2_AF_MODE  GPIO_AF7
#define USART2_PORT     GPIOA

#define USART2_TX       GPIO_PIN_2
#define USART2_RX       GPIO_PIN_3

// GPIO port/pin of hardware button on STM32f407G Disc. Board
#define BUTTON_PORT		GPIOA
#define BUTTON_PIN		GPIO_PIN_0

// command codes
#define REQ_LEN_CMD  0x51
#define SEND_MSG_CMD 0x52

// ====================== GLOBAL VARIABLES ======================

USART_handle_t usart2;
char RxBuf[256];

// global flags
volatile uint8_t Tx_Done = 0;
volatile uint8_t Rx_Done = 0;
volatile uint8_t failure = 0;

// ====================== END-GLOBAL VARIABLES ======================

// Initializes GPIO pins for USART2 peripheral
static void gpio_init()
{
    GPIO_handle_t gpio_usart2;
    GPIO_ConfigInit(&gpio_usart2.GPIO_PinConfig);

    gpio_usart2.pGPIOx = USART2_PORT;
    gpio_usart2.GPIO_PinConfig.mode           = GPIO_MODE_ALTFN;    // alternate-functionality mode
    gpio_usart2.GPIO_PinConfig.altFuncMode    = USART2_AF_MODE;     // set AF mode to usart2
	gpio_usart2.GPIO_PinConfig.outType        = GPIO_OTYPE_PP;      // push-pull
	gpio_usart2.GPIO_PinConfig.puPdCtl        = GPIO_NO_PUPD;
	gpio_usart2.GPIO_PinConfig.speed          = GPIO_SPEED_HIGH;

    // Tx
	gpio_usart2.GPIO_PinConfig.pinNumber      = USART2_TX;
    GPIO_Init(&gpio_usart2);

    // Rx
    gpio_usart2.GPIO_PinConfig.pinNumber      = USART2_RX;
    GPIO_Init(&gpio_usart2);
}

// Initialize USART2 peripheral
static void usart2_init()
{
    usart2.pUSARTx = USART2;
    USART_ConfigInit(&(usart2.USART_Config));

    status_t status = USART_Init(&usart2);
    printf("USART_Init Status: %d\n", status);
}

// helper function to reset usart2
static void reset_usart2()
{
    USART_Reset(usart2.pUSARTx);
    USART_Init(&usart2);
    failure = 0U; // reset failure flag
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
    usart2_init();
    gpio_init();

    // disable printf buffering
    setvbuf(stdout, NULL, _IONBF, 0);

    // enable IRQ number for USART2
    NVIC_EnableIRQ(IRQ_NUM_USART2);

    uint8_t n = 0U; // message length
    uint8_t cmd = REQ_LEN_CMD;
    status_t status;
    while(1)
    {
        wait_for_press();

        // clear flags & buffer
        Tx_Done = 0U;
        Rx_Done = 0U;
        failure = 0U;
        memset(RxBuf, 0, sizeof(RxBuf));

        // read message length
        Rx_Done = 0;
        status = USART_ReceiveDataIT(&usart2, &n, 1U); // arm reception before transmit
        if(status != STATUS_OK)
        {
            reset_usart2();
            continue;
        }

        // send REQ_LEN_CMD command code
        cmd = REQ_LEN_CMD;
        Tx_Done = 0;
        status = USART_SendDataIT(&usart2, &cmd, 1U);
        if(status != STATUS_OK)
        {
            reset_usart2();
            continue;
        }

        while((!Tx_Done || !Rx_Done) && !failure);
        if (failure)
        {
            reset_usart2();
            continue;
        }

        // read message
        Rx_Done = 0;
        status = USART_ReceiveDataIT(&usart2, RxBuf, n); // arm reception before Tx
        if(status != STATUS_OK)
        {
            reset_usart2();
            continue;
        }

        // request message
        Tx_Done = 0;
        cmd = SEND_MSG_CMD;
        status = USART_SendDataIT(&usart2, &cmd, 1U);
        if(status != STATUS_OK)
        {
            reset_usart2();
            continue;
        }

        while((!Tx_Done || !Rx_Done) && !failure);
        if (failure)
        {
            reset_usart2();
            continue;
        }

        // print message
        RxBuf[n] = '\0';
        printf("%s", RxBuf);
    }

    return 0;
}

void USART2_IRQHandler(void)
{
    USART_IRQHandling(&usart2);
}

void USART_ApplicationEventCallback(USART_handle_t *pHandle, USART_event_t event)
{
    if (event == USART_EVT_TX_DONE)
    {
        Tx_Done = 1;
    }
    else if (event == USART_EVT_RX_DONE)
    {
        Rx_Done = 1;
    }
    else
    {
        // printf("Error Event Detected: %d\n", event);
        failure = 1;
    }
}
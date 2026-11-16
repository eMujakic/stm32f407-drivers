/***************************************************************************
 * @file		014USART_Rx.c
 * @author		ernad
 * @date 		Nov 16, 2026
 * @brief		This demo uses the custom USART/UART drivers for the
 *              STM32F407 to receive a variable-length message from an
 *              Arduino Uno board.
 *
 * @details
 * Configures USART2 for Tx-Rx operation and configures PA2 and
 * PA3 for alternate function mode AF7. The onboard user button is
 * configured and triggers a sequence of USART transfers:
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
 * If any USART_SendData()/USART_ReceiveData() returns a non-ok status,
 * the USART2 peripheral is reset, reinitialized, and the current
 * transfer sequence is aborted. A new sequence begins on the next
 * button trigger.
 * 
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

    while(1)
    {
        wait_for_press();
        uint8_t len = 0;
        
        // send REQ_LEN_CMD
        uint8_t cmd = REQ_LEN_CMD;
        status_t status = USART_SendData(&usart2, &cmd, 1U);
        if(status != STATUS_OK)
        {
            reset_usart2();
            continue;
        }
        
        // Receive message length
        status = USART_ReceiveData(&usart2, &len, 1U);
        if(status != STATUS_OK)
        {
            reset_usart2();
            continue;
        }

        printf("Message Length: %d\n", len);

        // send SEND_MSG_CMD
        cmd = SEND_MSG_CMD;
        status = USART_SendData(&usart2, &cmd, 1U);
        if(status != STATUS_OK)
        {
            reset_usart2();
            continue;
        }
        
        // receive message
        status = USART_ReceiveData(&usart2, RxBuf, len);
        if(status != STATUS_OK)
        {
            reset_usart2();
            continue;
        }

        RxBuf[len] = '\0';
        printf("%s", RxBuf);
    }
    return 0;
}

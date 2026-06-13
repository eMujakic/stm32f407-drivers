/*
 * @file			003LED_button.c
 * @author	ernad
 * @date 		 Jun 12, 2026
 * @brief		LED button example using STM32F407G discovery board
 *
 * @details
 * this application demonstrates basic GPIO output functionality by
 * powering an on-board LED using the on-board user button on the
 * STM32F407G discovery board.
 */

#include "stm32f407xx.h"

#define LED_PORT 			GPIOD
#define LED_PIN				GPIO_PIN_13

#define BUTTON_PORT	GPIOA
#define BUTTON_PIN		GPIO_PIN_0

static void delay(uint32_t val)
{
	for(uint32_t i = 0; i < val; i++);
}

int main(void)
{
	// GPIO LED configuration and initialization
	GPIO_handle_t gpio_led;
	gpio_led.pGPIOx = LED_PORT;
	gpio_led.GPIO_PinConfig.pinNumber = LED_PIN;
	gpio_led.GPIO_PinConfig.mode = GPIO_MODE_OUT;
	gpio_led.GPIO_PinConfig.outType = GPIO_OTYPE_PP;
	gpio_led.GPIO_PinConfig.puPdCtl  = GPIO_NO_PUPD;
	gpio_led.GPIO_PinConfig.speed  = GPIO_SPEED_HIGH;

	GPIO_PeriClkCtl(LED_PORT, ENABLE);
	GPIO_Config(&gpio_led);

	// GPIO button configuration and initialization
	GPIO_handle_t gpio_button;
	gpio_led.pGPIOx = BUTTON_PORT;
	gpio_led.GPIO_PinConfig.pinNumber = BUTTON_PIN;
	gpio_led.GPIO_PinConfig.mode = GPIO_MODE_IN;
	gpio_led.GPIO_PinConfig.puPdCtl  = GPIO_NO_PUPD;		// no PD needed since it's is already on-board

	GPIO_PeriClkCtl(BUTTON_PORT, ENABLE);
	GPIO_Config(&gpio_button);

	while(1)
	{
			GPIO_WritePin(LED_PORT, LED_PIN, GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN));
			delay(500);
	}

	return 0;
}

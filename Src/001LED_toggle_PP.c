/*
 * @file			001LED_toggle.c
 * @author	ernad
 * @date 		 Jun 12, 2026
 * @brief		LED toggle example using STM32F407G discovery board
 *
 * @details
 * this application demonstrates basic GPIO output functionality in PUSH-
 * PULL configuration using the custom GPIO driver to target an LED on
 * port D, pin 13 on the STM32F407G discovery board.
 */
#include "stm32f407xx.h"

#define LED_PORT 	GPIOD
#define LED_PIN		GPIO_PIN_13

static void delay(uint32_t val)
{
	for(uint32_t i = 0; i < val; i++);
}

int main(void){

	GPIO_handle_t gpio_led;
	gpio_led.pGPIOx = LED_PORT;
	gpio_led.GPIO_PinConfig.pinNumber = LED_PIN;
	gpio_led.GPIO_PinConfig.mode = GPIO_MODE_OUT;
	gpio_led.GPIO_PinConfig.outType = GPIO_OTYPE_PP;
	gpio_led.GPIO_PinConfig.puPdCtl  = GPIO_NO_PUPD;
	gpio_led.GPIO_PinConfig.speed  = GPIO_SPEED_HIGH;

	GPIO_PeriClkCtl(LED_PORT, ENABLE);
	GPIO_Config(&gpio_led);

	while(1)
	{
		GPIO_ToggleOutputPin(LED_PORT, LED_PIN);
		delay(250000);
	}
	return 0;
}

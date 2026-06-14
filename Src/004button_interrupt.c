/*
 * @file		004button_interrupt.c
 * @author		ernad
 * @date 		Jun 14, 2026
 * @brief		Toggles on-board LED via interrupt delivered by button press
 *
 * @details
 * this application demonstrates basic GPIO interrupt delivery functionality
 * by toggling an on-board LED by having the on-board user button deliver an
 * interrupt. This project assumes that the external hardware (button & LED)
 * are connected in the same manner as the STM32F407G discovery board.
 */

#include "stm32f407xx.h"

#define LED_PORT 			GPIOD
#define LED_PIN				GPIO_PIN_12

#define BUTTON_PORT			GPIOA
#define BUTTON_PIN			GPIO_PIN_0

static void delay(uint32_t val)
{
	for(uint32_t i = 0; i < val; i++);
}

int main(void)
{
	// GPIO LED configuration and initialization
	GPIO_handle_t gpio_led;
	GPIO_init(&gpio_led.GPIO_PinConfig);
	gpio_led.pGPIOx = LED_PORT;
	gpio_led.GPIO_PinConfig.pinNumber = LED_PIN;
	gpio_led.GPIO_PinConfig.mode = GPIO_MODE_OUT;

	GPIO_PeriClkCtl(LED_PORT, ENABLE);
	GPIO_Config(&gpio_led);

	// GPIO button configuration and initialization
	GPIO_handle_t gpio_button;
	IRQ_num_t IRQ_num =  GPIO_PinToIRQNumber(BUTTON_PIN);
	GPIO_init(&gpio_button.GPIO_PinConfig);
	gpio_button.pGPIOx = BUTTON_PORT;
	gpio_button.GPIO_PinConfig.pinNumber = BUTTON_PIN;
	gpio_button.GPIO_PinConfig.mode = GPIO_MODE_IT_RT;		// rising-trigger interrupt mode

	// IRQ configurations
	GPIO_IRQEnable(IRQ_num);
	GPIO_IRQPriority(IRQ_num, IRQ_PRI15);

	GPIO_PeriClkCtl(BUTTON_PORT, ENABLE);
	GPIO_Config(&gpio_button);

	while(1);

	return 0;
}

void EXTI0_IRQHandler(void)
{
	delay(100*1500);						// for debouncing
	GPIO_IRQHandling(BUTTON_PIN);			// clears pending interrupt from EXTI line
	GPIO_ToggleOutputPin(LED_PORT, LED_PIN);
}

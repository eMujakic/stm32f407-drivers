/*
 * stm32f407xx_gpio_driver.h
 *
 *  Created on: Jun 11, 2026
 *      Author: ernad
 */

#ifndef INC_STM32F407XX_GPIO_DRIVER_H_
#define INC_STM32F407XX_GPIO_DRIVER_H_

#include "stm32f407xx.h"

// Configuration structure for a GPIO pin
typedef struct
{
	uint8_t pinNumber;
	uint8_t pinMode;
	uint8_t pinSpeed;
	uint8_t pinPinPuPdCtl;
	uint8_t pinOType;
	uint8_t pinAltFuncMode;
} GPIO_PinConfig_t;

// Handle structure for a GPIO pin
typedef struct
{
	GPIO_REG_T *pGPIOx;								// pointer to base addr of GPIO port
	GPIO_PinConfig_t GPIO_PinConfig;		// holds GPIO pin config settings
} GPIO_handle_t;

/*****************************************************************************
 * 																APIs supported by this driver
 * 												Check function definitions for more information
 ******************************************************************************/

void GPIO_init(GPIO_handle_t *pGPIOHandle);																//
void GPIO_Reset(GPIO_REG_T *pGPIOx);																			//

void GPIO_PeriClkCtl(GPIO_REG_T *pGPIOx, uint8_t enable);										// can enable/disable the clock for a given GPIO base addr

uint8_t GPIO_ReadPin(GPIO_REG_T *pGPIOx, uint8_t pinNumber);							// TODO
uint16_t GPIO_ReadPort(GPIO_REG_T *pGPIOx);															// TODO

void GPIO_WritePin(GPIO_REG_T *pGPIOx, uint8_t pinNumber, uint8_t val);			// TODO
void GPIO_WritePort(GPIO_REG_T *pGPIOx, uint16_t val);											// TODO
void GPIO_ToggleOutputPin(GPIO_REG_T *pGPIOx, uint8_t pinNumber);				// TODO

void GPIO_IRQConfig(uint8_t IRQNumber, uint8_t IRQPriority, uint8_t enable);		//
void GPIO_IRQHandling(uint8_t pinNumber);																	//








#endif /* INC_STM32F407XX_GPIO_DRIVER_H_ */

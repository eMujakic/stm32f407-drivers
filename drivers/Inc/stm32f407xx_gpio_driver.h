/*
 * stm32f407xx_gpio_driver.h
 *
 *  Created on: Jun 11, 2026
 *      Author: ernad
 */

#ifndef INC_STM32F407XX_GPIO_DRIVER_H_
#define INC_STM32F407XX_GPIO_DRIVER_H_

#include "stm32f407xx.h"

// ====================== GPIO pin mode enum ======================
typedef enum
{
	GPIO_MODE_IN = 0,
	GPIO_MODE_OUT,
	GPIO_MODE_ALTFN,						// alternate-function mode
	GPIO_MODE_ANALOG
} GPIO_mode_t;

// ====================== GPIO pin interrupt mode enum ======================
typedef enum
{
	GPIO_MODE_IT_FT = 4,					// interrupt falling-edge trigger
	GPIO_MODE_IT_RT,						// interrupt rising-edge trigger
	GPIO_MODE_IT_RFT						// interrupt rising & falling-edge trigger
} GPIO_it_mode_t;

// ====================== GPIO output type enum ======================
typedef enum
{
	GPIO_OTYPE_PP	 = 0,						// push-pull
	GPIO_OTYPE_OD							// open-drain
} GPIO_out_t;

// ====================== GPIO speed type enum ======================
typedef enum
{
	GPIO_SPEED_LOW = 0,
	GPIO_SPEED_MED,
	GPIO_SPEED_HIGH,
	GPIO_SPEED_VHIGH						// very-high
} GPIO_speed_t;

// ====================== GPIO pull-up / pull-down enum ======================
typedef enum
{
	GPIO_NO_PUPD = 0,						// no pull-up / pull-down
	GPIO_PIN_PU,								// pull-up
	GPIO_PIN_PD									// pull-down
} GPIO_PUPD_t;

// ====================== GPIO pin number enum ======================
typedef enum
{
	GPIO_PIN_0 = 0,
	GPIO_PIN_1,
	GPIO_PIN_2,
	GPIO_PIN_3,
	GPIO_PIN_4,
	GPIO_PIN_5,
	GPIO_PIN_6,
	GPIO_PIN_7,
	GPIO_PIN_8,
	GPIO_PIN_9,
	GPIO_PIN_10,
	GPIO_PIN_11,
	GPIO_PIN_12,
	GPIO_PIN_13,
	GPIO_PIN_14,
	GPIO_PIN_15,
} GPIO_pin_number_t;

// ====================== Configuration structure for a GPIO pin ======================
typedef struct
{
	GPIO_pin_number_t 	pinNumber;
	GPIO_mode_t 					mode;
	GPIO_speed_t 				speed;
    GPIO_PUPD_t 					puPdCtl;
	GPIO_out_t 						outType;
	uint8_t 								altFuncMode;
} GPIO_pin_config_t;

// ====================== Handle structure for a GPIO pin ======================
typedef struct
{
	GPIO_reg_t *pGPIOx;								// pointer to base addr of GPIO port
	GPIO_pin_config_t GPIO_PinConfig;		// holds GPIO pin config settings
} GPIO_handle_t;

/*****************************************************************************
 * 																APIs supported by this driver
 * 												Check function definitions for more information
 ******************************************************************************/

status_t GPIO_init(GPIO_handle_t *pGPIOHandle);																// TODO
status_t GPIO_Reset(GPIO_reg_t *pGPIOx);																				// TODO

status_t GPIO_PeriClkCtl(GPIO_reg_t *pGPIOx, uint8_t enable);										// can enable/disable the clock for a given GPIO base addr

uint8_t GPIO_ReadPin(GPIO_reg_t *pGPIOx, uint8_t pinNumber);										// TODO
uint16_t GPIO_ReadPort(GPIO_reg_t *pGPIOx);																		// TODO

status_t GPIO_WritePin(GPIO_reg_t *pGPIOx, uint8_t pinNumber, uint8_t val);			// TODO
status_t GPIO_WritePort(GPIO_reg_t *pGPIOx, uint16_t val);												// TODO
status_t GPIO_ToggleOutputPin(GPIO_reg_t *pGPIOx, uint8_t pinNumber);					// TODO

status_t GPIO_IRQConfig(uint8_t IRQNumber, uint8_t IRQPriority, uint8_t enable);		// TODO
status_t GPIO_IRQHandling(uint8_t pinNumber);																	// TODO








#endif /* INC_STM32F407XX_GPIO_DRIVER_H_ */

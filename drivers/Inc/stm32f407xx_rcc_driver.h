/*
 * stm32f407xx_rcc_driver.h
 *
 *  Created on: Oct 29, 2026
 *      Author: ernad
 */

#ifndef INC_STM32F407XX_RCC_DRIVER_H_
#define INC_STM32F407XX_RCC_DRIVER_H_

#include "stm32f407xx.h"

#define RCC_HZ_PER_KHZ      1000U
#define RCC_HZ_PER_MHZ      1000000U
#define RCC_HZ_PER_GHZ      1000000000U

#define RCC_HSI_HZ 16000000U	// 16Hz HSI
#define RCC_HSE_HZ 8000000U		// 8Hz HSE on STM32F407G Disc. Board

/*********************************************************************************
 * 							APIs supported by this driver
 * 					Check function definitions for more information
 *********************************************************************************/

uint32_t RCC_GetSYSCLKspeed(void);          // Returns speed of device SYSCLK in Hz
uint32_t RCC_GetHCLKSpeed(void);            // Returns speed of device HCLK (SYSCLK / AHB_Prescaler) in Hz

uint32_t RCC_HzToMHz(uint32_t hz);          // Returns the number of MHz in hz

// uint8_t RCC_GetAPB1Prescaler(void);         //
uint32_t RCC_GetPCLK1Speed(void);           //

#endif /* INC_STM32F407XX_RCC_DRIVER_H_ */

/*
 * stm32f407xx_nvic_driver.h
 *
 *  Created on: Aug 22, 2026
 *      Author: ernad
 */

#ifndef INC_STM32F407XX_NVIC_DRIVER_H_
#define INC_STM32F407XX_NVIC_DRIVER_H_

#include "stm32f407xx.h"

/*********************************************************************************
 * 							APIs supported by this driver
 * 					Check function definitions for more information
 *********************************************************************************/

status_t NVIC_EnableIRQ(IRQ_num_t IRQNumber);                               // Enables a peripheral interrupt in the NVIC.
status_t NVIC_DisableIRQ(IRQ_num_t IRQNumber);                              // Disables a peripheral interrupt in the NVIC.

status_t NVIC_SetPriority(IRQ_num_t IRQNumber, IRQ_priority_t priority);    // Sets the priority level for an IRQ number in the NVIC.

// status_t NVIC_SetPendingIRQ(IRQ_num_t IRQNumber);
// status_t NVIC_ClearPendingIRQ(IRQ_num_t IRQNumber);

#endif /* INC_STM32F407XX_NVIC_DRIVER_H_ */

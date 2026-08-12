/*
 * stm32f407xx_gpio_driver.c
 *
 *  Created on: Jun 11, 2026
 *      Author: ernad
 */

#include "stm32f407xx_gpio_driver.h"

/**************************************************************************
 * @fn		GPIO_write_field
 *
 * @brief	Writes a value to a specific bit-field of a GPIO register.
 *
 * @details
 * Performs a read-modify-write by masking and shifting the target
 * field corresponding to the specified GPIO pin number.
 *
 * @param[in] reg		Pointer to the target 32-bit hardware register.
 * @param[in] pin		GPIO pin number used to calculate the field position.
 * @param[in] width		Width in bits of each pin's field in the register.
 * @param[in] attr		Value to be written into the selected bit field.
 *
 * @return	None.
 *
 * @note	Assume the register layout is uniform per pin.
 ***************************************************************************/
static inline void GPIO_write_field(volatile uint32_t *reg, GPIO_pin_number_t pin, uint32_t width, uint32_t attr)
{
	uint32_t shift 	= width * pin;
	uint32_t mask 	= ((1U << width) - 1U) << shift;
	attr 			&= ((1U << width) - 1U) ;		// mask attr with only the first "width" bits

	// clear current register bits
	*reg &= ~mask;

	// set new register bits
	*reg |= (attr << shift);
}


 /**************************************************************************
 * @fn		GPIO_ConfigInit
 *
 * @brief	Initializes a GPIO_pin_config_t struct with safe default values.
 *
 * @details
 * Sets the configuration to:
 *		- Pin 0
 *		- Input mode
 *		- Low speed
 *		- No pull-up/pull-down
 *		- Push-pull output type
 *		- Alternate function 0
 *
 * @param[in] pConfig	A pointer to the configuration struct to be initialized.
 *
 * @return	None.
 *
 * @note	None.
 ***************************************************************************/
status_t GPIO_ConfigInit(GPIO_pin_config_t *pConfig)
{
	if(!pConfig) return STATUS_INVALID_PARAM;

    pConfig->pinNumber  = GPIO_PIN_0;
    pConfig->mode       = GPIO_MODE_IN;
    pConfig->speed      = GPIO_SPEED_LOW;
    pConfig->puPdCtl    = GPIO_NO_PUPD;
    pConfig->outType    = GPIO_OTYPE_PP;
    pConfig->altFuncMode = GPIO_AF0;

	return STATUS_OK;
}

 /**************************************************************************
 * @fn		GPIO_port_to_idx
 *
 * @brief	Converts a GPIO base address to its corresponding 0-based index.
 *
 * @param[in] GPIOx		Pointer to the base address of the GPIO peripheral.
 *
 * @return
 *		0-10	: Valid GPIO port index.
 * 		-1		: Invalid GPIO port pointer.
 *
 * @note	The returned value corresponds to the encoding required for
 *			SYSCFG->EXTICR bit-fields, as well as GPIO clock enable/disable
 *			macros.
 ***************************************************************************/
static int8_t GPIO_port_to_idx(GPIO_reg_t* pGPIOx)
{
	if(pGPIOx == GPIOA)
		return 0;
	else if (pGPIOx == GPIOB)
		return 1;
	else if (pGPIOx == GPIOC)
		return 2;
	else if (pGPIOx == GPIOD)
		return 3;
	else if (pGPIOx == GPIOE)
		return 4;
	else if (pGPIOx == GPIOF)
		return 5;
	else if (pGPIOx == GPIOG)
		return 6;
	else if (pGPIOx == GPIOH)
		return 7;
	else if (pGPIOx == GPIOI)
		return 8;
	else if (pGPIOx == GPIOJ)
		return 9;
	else if (pGPIOx == GPIOK)
		return 10;
	else return -1;
}

 /**************************************************************************
 * @fn		GPIO_Init
 *
 * @brief	Initializes a given GPIO pin according to the config settings
 *			in the handle.
 *
 * @param[in] pGPIOHandle		Pointer to a GPIO handle structure
 * 								containing the GPIO port base address and
 *								configuration parameters.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_ERROR / STATUS_INVALID_PARAM).
 *
 * @note	None.
 ***************************************************************************/
status_t GPIO_Init(GPIO_handle_t *pGPIOHandle)
{
	if(!pGPIOHandle) return STATUS_INVALID_PARAM;					// validate handle

	uint32_t pinNumber 	= pGPIOHandle->GPIO_PinConfig.pinNumber;
	if(pinNumber > GPIO_PIN_15) return STATUS_INVALID_PARAM;		// validate pin number

	GPIO_mode_t mode = pGPIOHandle->GPIO_PinConfig.mode;

	// ======================== 1. Configure the mode of GPIO pin ========================
	if(mode <= GPIO_MODE_ANALOG)	// non-interrupt modes
	{
		GPIO_write_field(&pGPIOHandle->pGPIOx->MODER, pinNumber, 2U, mode);
	}
	else							// interrupt modes
	{
		if(mode == GPIO_MODE_IT_FT)
		{
			// set corresponding bit in FTSR register
			EXTI->FTSR |= (1U << pinNumber);

			// clear corresponding bit in RTSR register
			EXTI->RTSR &= ~(1U << pinNumber);
		}
		else if(mode == GPIO_MODE_IT_RT)
		{
			// set corresponding bit in RTSR register
			EXTI->RTSR |= (1U << pinNumber);

			// clear corresponding bit in FTSR register
			EXTI->FTSR &= ~(1U << pinNumber);
		}
		else if (mode == GPIO_MODE_IT_RFT)
		{
			// set corresponding bits in RTSR  and FTSR registers
			EXTI->RTSR |= (1U << pinNumber);
			EXTI->FTSR |= (1U << pinNumber);
		}

		// configure GPIO port selection in SYSCFG_EXTICR
		uint8_t EXTI_CR_idx 	= pinNumber / 4U;
		uint8_t offset			= (pinNumber % 4U) * 4U;
		uint32_t idx			= (uint32_t) (0xFU & (GPIO_port_to_idx(pGPIOHandle->pGPIOx)));

		SYSCFG_PCLK_EN();	// enable SYSCFG peripheral clock

		// clear previous EXTICR bits
		SYSCFG->EXTICR[EXTI_CR_idx] &= ~(0xFU << offset);

		// set corresponding EXTICR bits
		SYSCFG->EXTICR[EXTI_CR_idx] |= (idx << offset);

		// enable EXTI interrupt delivery using IMR
		EXTI->IMR |= (1U << pinNumber);
	}

	// ======================== 2. Configure the speed of GPIO pin ========================
	if(mode == GPIO_MODE_OUT || mode == GPIO_MODE_ALTFN)
	{
		GPIO_write_field(&pGPIOHandle->pGPIOx->OSPEEDR, pinNumber, 2U, pGPIOHandle->GPIO_PinConfig.speed);
	}

	// 3. ======================== Configure pull-up/pull-down settings of GPIO pin ========================
	GPIO_write_field(&pGPIOHandle->pGPIOx->PUPDR, pinNumber, 2U, pGPIOHandle->GPIO_PinConfig.puPdCtl);

	// 4. ======================== Configure the output type ========================
	if(mode == GPIO_MODE_OUT ||mode == GPIO_MODE_ALTFN)
	{
		GPIO_write_field(&pGPIOHandle->pGPIOx->OTYPER, pinNumber, 1U, pGPIOHandle->GPIO_PinConfig.outType);
	}

	// 4. ======================== Configure the alternate functionality ========================
	if(mode == GPIO_MODE_ALTFN)
	{
		uint8_t afrIdx		= (uint8_t) (pinNumber / 8U);
		uint32_t offset 	= (uint32_t) (pinNumber % 8);

		GPIO_write_field(&pGPIOHandle->pGPIOx->AFR[afrIdx], offset, 4U, pGPIOHandle->GPIO_PinConfig.altFuncMode);
	}

	return STATUS_OK;
}

 /**************************************************************************
 * @fn		GPIO_Reset
 *
 * @brief	This function resets a given GPIO port to default settings
 *
 * @param[in] pGPIOx		Pointer to the base address of the GPIO peripheral.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_ERROR / STATUS_INVALID_PARAM).
 *
 * @note	None.
 ***************************************************************************/
status_t GPIO_Reset(GPIO_reg_t *pGPIOx)
{
	if (! pGPIOx) return STATUS_INVALID_PARAM;

	int8_t idx = GPIO_port_to_idx(pGPIOx);
	if(idx < 0) return STATUS_INVALID_PARAM;

	GPIOx_RESET(idx);

	return STATUS_OK;
}

 /**************************************************************************
 * @fn		GPIO_PeriClkCtl
 *
 * @brief	Enables or disables the peripheral clock for a given GPIO port.
 *
 * @param[in] pGPIOx		Pointer to the base address of the GPIO peripheral.
 * @param[in] enable		ENABLE or DISABLE macro.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_ERROR / STATUS_INVALID_PARAM).
 *
 * @note	None.
 ***************************************************************************/
status_t GPIO_PeriClkCtl(GPIO_reg_t *pGPIOx, uint8_t enable)
{
	if (!pGPIOx) return STATUS_INVALID_PARAM;

	int8_t idx = GPIO_port_to_idx(pGPIOx);
	if(idx < 0) return STATUS_INVALID_PARAM;

	if(enable)
		GPIOx_PCLK_EN( idx );
	else
		GPIOx_PCLK_DI( idx );

	return STATUS_OK;
}

 /**************************************************************************
 * @fn		GPIO_ReadPin
 *
 * @brief	Reads the input register for a given GPIO pin.
 *
 * @param[in] pGPIOx		Pointer to the base address of the GPIO peripheral.
 * @param[in] pinNumber		Pin number to read input data from.
 *
 * @return	The data (1 or 0) in the input data register of the corresponding
			GPIO port & pin.
 *
 * @note	None.
 ***************************************************************************/
uint8_t GPIO_ReadPin(GPIO_reg_t *pGPIOx, uint8_t pinNumber)
{
	if(pinNumber > GPIO_PIN_15 || ! pGPIOx) return 0;

	return (uint8_t) ( (pGPIOx->IDR >> pinNumber) & 1U );
}

 /**************************************************************************
 * @fn		GPIO_ReadPort
 *
 * @brief	Reads the entire input register for a given GPIO port.
 *
 * @param[in] pGPIOx		Pointer to the base address of the GPIO peripheral.
 *
 * @return	The entire input data register of the corresponding GPIO port.
 *
 * @note	None.
 ***************************************************************************/
uint16_t GPIO_ReadPort(GPIO_reg_t *pGPIOx)
{
	if(! pGPIOx) return 0;

	return (uint16_t) pGPIOx->IDR;
}

 /**************************************************************************
 * @fn		GPIO_WritePin
 *
 * @brief	Writes the output register for a given GPIO pin.
 *
 * @param[in] pGPIOx		Pointer to the base address of the GPIO peripheral.
 * @param[in] pinNumber		Pin number to write output data to.
 * @param[in] val			The value to be written (SET or RESET).
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_ERROR / STATUS_INVALID_PARAM).
 *
 * @note	None.
 ***************************************************************************/
status_t GPIO_WritePin(GPIO_reg_t *pGPIOx, uint8_t pinNumber, uint8_t val)
{
	if(pinNumber > GPIO_PIN_15 || ! pGPIOx) return STATUS_INVALID_PARAM;

	// uses BSRR register for atomic bit set/reset
	pGPIOx->BSRR = (val == SET)
	                    ? (1U << pinNumber)				// SET bit
	                    : (1U << (pinNumber + 16U));	// RESET bit

	return STATUS_OK;
}

 /**************************************************************************
 * @fn		GPIO_WritePort
 *
 * @brief	Writes the entire output register for a given GPIO port.
 *
 * @param[in] pGPIOx		Pointer to the base address of the GPIO peripheral.
 * @param[in] val			The value to be written (entire register).
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_ERROR / STATUS_INVALID_PARAM).
 *
 * @note	None.
 ***************************************************************************/
status_t GPIO_WritePort(GPIO_reg_t *pGPIOx, uint16_t val)
{
	if(! pGPIOx) return STATUS_INVALID_PARAM;

	pGPIOx->ODR = (uint32_t) val;

	return STATUS_OK;
}

 /**************************************************************************
 * @fn		GPIO_ToggleOutputPin
 *
 * @brief	Toggles the output value of a given GPIO pin.
 *
 * @param[in] pGPIOx		Pointer to the base address of the GPIO peripheral.
 * @param[in] pinNumber		The pin to toggle.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_ERROR / STATUS_INVALID_PARAM).
 *
 * @note	None.
 ***************************************************************************/
status_t GPIO_ToggleOutputPin(GPIO_reg_t *pGPIOx, uint8_t pinNumber)
{
	if(pinNumber > GPIO_PIN_15 || ! pGPIOx) return STATUS_INVALID_PARAM;

	pGPIOx->BSRR = ( pGPIOx->ODR & (1U << pinNumber) )
		                    ? (1U << (pinNumber + 16U))		// if HIGH then RESET
		                    : (1U << pinNumber);			// if LOW then SET

	return STATUS_OK;
}

/**************************************************************************
 * @fn		GPIO_PinToIRQNumber
 *
 * @brief	Returns the IRQ number for a given GPIO pin.
 *
 * @param[in] pin		The GPIO pin number.
 *
 * @return	The IRQ number corresponding to the given GPIO pin.
 *
 * @note	None.
 ***************************************************************************/
IRQ_num_t 	GPIO_PinToIRQNumber(GPIO_pin_number_t pin)
{
	if(pin >= GPIO_PIN_0 && pin <= GPIO_PIN_4 ) return (IRQ_num_t)(IRQ_NUM_EXTI0 + pin);

	if(pin >= GPIO_PIN_5 && pin <= GPIO_PIN_9 ) return IRQ_NUM_EXTI5_9;

	if(pin >= GPIO_PIN_10 && pin <= GPIO_PIN_15 ) return IRQ_NUM_EXTI10_15;

	return IRQ_NUM_INVALID;
}

 /**************************************************************************
 * @fn		GPIO_IRQEnable
 *
 * @brief	Enables a given IRQNumber in the NVIC of the processor (ARM Cortex M4).
 *
 * @param[in] IRQNumber		The IRQ number to enable in the NVIC.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_ERROR / STATUS_INVALID_PARAM).
 *
 * @note	None.
 ***************************************************************************/
status_t GPIO_IRQEnable(uint8_t IRQNumber)
{
	NVIC_ISER[IRQNumber / 32U] = (1U << (IRQNumber % 32U));		// uses write instead of read-modify-write
	return STATUS_OK;
}

 /**************************************************************************
 * @fn		GPIO_IRQDisable
 *
 * @brief	Disables a given IRQNumber in the NVIC of the processor (ARM Cortex M4).
 *
 * @param[in] IRQNumber		The IRQ number to disable in the NVIC.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_ERROR / STATUS_INVALID_PARAM).
 *
 * @note	None.
 ***************************************************************************/
status_t GPIO_IRQDisable(uint8_t IRQNumber)
{
	NVIC_ICER[IRQNumber / 32U] = (1U << (IRQNumber % 32U));		// uses write instead of read-modify-write
	return STATUS_OK;
}

 /**************************************************************************
 * @fn		GPIO_IRQPriority
 *
 * @brief	Sets the priority of an IRQNumber in the NVIC.
 *
 * @param[in] IRQNumber		The target IRQ number.
 * @param[in] IRQPriority	The priority value to set the IRQ number to.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_ERROR / STATUS_INVALID_PARAM).
 *
 * @note	None.
 ***************************************************************************/
status_t GPIO_IRQPriority(uint8_t IRQNumber, uint8_t IRQPriority)
{
	uint32_t reg_idx 	= IRQNumber / 4;
	uint32_t offset 	= ((IRQNumber % 4) * 8U);

	// Mask priority to implemented priority bits
	// (e.g., 0x0FU if NO_PR_BITS_IMPLEMENTED == 4)
	IRQPriority &= ((1U << NO_PR_BITS_IMPLEMENTED) - 1U);

	// clear current priority bits
	NVIC_IPR[reg_idx] &= ~(0xFFU << offset);
	// set new priority bits
	NVIC_IPR[reg_idx] |= (IRQPriority << (offset + (8U - NO_PR_BITS_IMPLEMENTED)));	// bits must be shifted to upper nibble

	return STATUS_OK;
}

 /**************************************************************************
 * @fn		GPIO_IRQHandling
 *
 * @brief	Clears the pending interrupt bit in the EXTI peripheral
			for the specified GPIO pin.
 *
 * @param[in] pinNumber		The target pin number / EXTI line (0-15).
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_INVALID_PARAM).
 *
 * @note	None.
 ***************************************************************************/
status_t GPIO_IRQHandling(uint8_t pinNumber)
{
	if (pinNumber > GPIO_PIN_15) return STATUS_INVALID_PARAM;

	// clear the corresponding EXTI PR register
	EXTI->PR = (1U << pinNumber);	// bit is cleared by writing 1

	return STATUS_OK;
}



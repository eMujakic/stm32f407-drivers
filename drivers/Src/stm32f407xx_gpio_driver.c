/*
 * stm32f407xx_gpio_driver.c
 *
 *  Created on: Jun 11, 2026
 *      Author: ernad
 */

#include "stm32f407xx_gpio_driver.h"

static void inline GPIO_write_field(volatile uint32_t *reg, GPIO_pin_number_t pin, uint32_t width, uint32_t attr)
{
	uint32_t shift 	= width * pin;
	uint32_t mask = ((1U << width) - 1U) << shift;
	attr 					 &= ((1U << width) - 1U) ;					// mask attr with only the first "width" bits

	// clear current register bits
	*reg &= ~mask;

	// set new register bits
	*reg |= (attr << shift);
}

/*********************************************************************
 * @fn      		 			- GPIO_init
 *
 * @brief             		- This function initializes a give GPIO pin according to the config settings in the handle
 *
 * @param[in]       	- pointer to a struct containing GPIO port pointer and config struct
 *
 * @return           		-  status indicating whether the command was successful or not
 *
 * @Note            		-  none
 **********************************************************************/
status_t GPIO_init(GPIO_handle_t *pGPIOHandle)
{
	if(!pGPIOHandle) return STATUS_INVALID_PARAM;											// validate handle

	uint32_t pinNumber 	= pGPIOHandle->GPIO_PinConfig.pinNumber;
	if(pinNumber > GPIO_PIN_15) return STATUS_INVALID_PARAM;						// validate pin number

	// ======================== 1. Configure the mode of GPIO pin ========================
	if(pGPIOHandle->GPIO_PinConfig.mode <= GPIO_MODE_ANALOG)		// non-interrupt modes
	{
		GPIO_write_field(&pGPIOHandle->pGPIOx->MODER, pinNumber, 2U, pGPIOHandle->GPIO_PinConfig.mode);
	}
	else	// interrupt modes
	{
			// TODO set interrupt modes
	}

	// ======================== 2. Configure the speed of GPIO pin ========================
	if(pGPIOHandle->GPIO_PinConfig.mode == GPIO_MODE_OUT ||
			pGPIOHandle->GPIO_PinConfig.mode == GPIO_MODE_ALTFN)
	{
		GPIO_write_field(&pGPIOHandle->pGPIOx->OSPEEDR, pinNumber, 2U, pGPIOHandle->GPIO_PinConfig.speed);
	}

	// 3. ======================== Configure pull-up/pull-down settings of GPIO pin ========================
	GPIO_write_field(&pGPIOHandle->pGPIOx->PUPDR, pinNumber, 2U, pGPIOHandle->GPIO_PinConfig.puPdCtl);

	// 4. ======================== Configure the output type ========================
	if(pGPIOHandle->GPIO_PinConfig.mode == GPIO_MODE_OUT ||
		pGPIOHandle->GPIO_PinConfig.mode == GPIO_MODE_ALTFN)
	{
		GPIO_write_field(&pGPIOHandle->pGPIOx->OTYPER, pinNumber, 1U, pGPIOHandle->GPIO_PinConfig.outType);
	}

	// 4. ======================== Configure the alternate functionality ========================
	if(pGPIOHandle->GPIO_PinConfig.mode == GPIO_MODE_ALTFN)
	{
		uint8_t afrIdx		= (uint8_t) (pinNumber / 8U);
		uint32_t offset 	= (uint32_t) (pinNumber % 8);

		GPIO_write_field(&pGPIOHandle->pGPIOx->AFR[afrIdx], offset, 4U, pGPIOHandle->GPIO_PinConfig.altFuncMode);
	}

	return STATUS_OK;
}

/*********************************************************************
 * @fn      		 			- GPIO_Reset
 *
 * @brief             		- This function resets a given GPIO port to default settings
 *
 * @param[in]       	- base address of the GPIO port peripheral
 *
 * @return           		-  status indicating whether the command was successful or not
 *
 * @Note            		-  none
 **********************************************************************/
status_t GPIO_Reset(GPIO_reg_t *pGPIOx)
{
	if(pGPIOx == GPIOA)
		GPIOA_RESET();
	else if(pGPIOx == GPIOB)
		GPIOB_RESET();
	else if(pGPIOx == GPIOC)
		GPIOC_RESET();
	else if(pGPIOx == GPIOD)
		GPIOD_RESET();
	else if(pGPIOx == GPIOE)
		GPIOE_RESET();
	else if(pGPIOx == GPIOF)
		GPIOF_RESET();
	else if(pGPIOx == GPIOG)
		GPIOG_RESET();
	else if(pGPIOx == GPIOH)
		GPIOH_RESET();
	else if(pGPIOx == GPIOI)
		GPIOI_RESET();
	else
		return STATUS_INVALID_PARAM;

	return STATUS_OK;
}

/*********************************************************************
 * @fn      		 			- GPIO_PeriClkCtl
 *
 * @brief             		- This function enables or disables peripheral clock for the given GPIO port
 *
 * @param[in]       	- base address of the GPIO port peripheral
 * @param[in]        	- ENABLE or DISABLE macro
 *
 * @return           		-  status indicating whether the command was successful or not
 *
 * @Note            		-  none
 **********************************************************************/
status_t GPIO_PeriClkCtl(GPIO_reg_t *pGPIOx, uint8_t enable)
{
	if (! pGPIOx) return STATUS_INVALID_PARAM;

	if(enable){
		if(pGPIOx == GPIOA)
			GPIOA_PCLK_EN();
		else if(pGPIOx == GPIOB)
			GPIOB_PCLK_EN();
		else if(pGPIOx == GPIOC)
			GPIOC_PCLK_EN();
		else if(pGPIOx == GPIOD)
			GPIOD_PCLK_EN();
		else if(pGPIOx == GPIOE)
			GPIOE_PCLK_EN();
		else if(pGPIOx == GPIOF)
			GPIOF_PCLK_EN();
		else if(pGPIOx == GPIOG)
			GPIOG_PCLK_EN();
		else if(pGPIOx == GPIOH)
			GPIOH_PCLK_EN();
		else if(pGPIOx == GPIOI)
			GPIOI_PCLK_EN();
		else
			return STATUS_INVALID_PARAM;
	}
	else
	{
		if(pGPIOx == GPIOA)
			GPIOA_PCLK_DI();
		else if(pGPIOx == GPIOB)
			GPIOB_PCLK_DI();
		else if(pGPIOx == GPIOC)
			GPIOC_PCLK_DI();
		else if(pGPIOx == GPIOD)
			GPIOD_PCLK_DI();
		else if(pGPIOx == GPIOE)
			GPIOE_PCLK_DI();
		else if(pGPIOx == GPIOF)
			GPIOF_PCLK_DI();
		else if(pGPIOx == GPIOG)
			GPIOG_PCLK_DI();
		else if(pGPIOx == GPIOH)
			GPIOH_PCLK_DI();
		else if(pGPIOx == GPIOI)
			GPIOI_PCLK_DI();
		else
			return STATUS_INVALID_PARAM;
	}

	return STATUS_OK;
}

/*********************************************************************
 * @fn      		 			- GPIO_ReadPin
 *
 * @brief             		- This function reads the input register for a given GPIO pin
 *
 * @param[in]       	- base address of the GPIO port peripheral
 * @param[in]        	- pin number to read value from
 *
 * @return           		-  returns the read value from the input register (0 or 1)
 *
 * @Note            		-  none
 **********************************************************************/
uint8_t GPIO_ReadPin(GPIO_reg_t *pGPIOx, uint8_t pinNumber)
{
	if(pinNumber > GPIO_PIN_15 || ! pGPIOx) return 0;

	return (uint8_t) ( (pGPIOx->IDR >> pinNumber) & 1U );
}

/*********************************************************************
 * @fn      		 			- GPIO_ReadPort
 *
 * @brief             		- This function reads the entire input register for a given GPIO port
 *
 * @param[in]       	- base address of the GPIO port peripheral
 *
 * @return           		-  the contents of the entire input register for the requested GPIO port
 *
 * @Note            		-  none
 **********************************************************************/
uint16_t GPIO_ReadPort(GPIO_reg_t *pGPIOx)
{
	if(! pGPIOx) return 0;

	return (uint16_t) pGPIOx->IDR;
}

/*********************************************************************
 * @fn      		 			- GPIO_WritePin
 *
 * @brief             		- This function writes to the output register of the requested GPIO pin
 *
 * @param[in]       	- base address of the GPIO port peripheral
 * @param[in]        	- the number of the pin to write to
 * @param[in]			- the value to be written using the SET / RESET macro
 *
 * @return           		-  status indicating whether the command was successful or not
 *
 * @Note            		-  none
 **********************************************************************/
status_t GPIO_WritePin(GPIO_reg_t *pGPIOx, uint8_t pinNumber, uint8_t val)
{
	if(pinNumber > GPIO_PIN_15 || ! pGPIOx) return STATUS_INVALID_PARAM;

	// uses BSRR register for atomic bit set/reset
	pGPIOx->BSRR = (val == SET)
	                    ? (1U << pinNumber)					// SET bit
	                    : (1U << (pinNumber + 16U));	// RESET bit

	return STATUS_OK;
}

/*********************************************************************
 * @fn      		 			- GPIO_WritePort
 *
 * @brief             		- This function writes the entire output register of a given GPIO port
 *
 * @param[in]       	- base address of the GPIO port peripheral
 *
 * @return           		-  status indicating whether the command was successful or not
 *
 * @Note            		-  none
 **********************************************************************/
status_t GPIO_WritePort(GPIO_reg_t *pGPIOx, uint16_t val)
{
	if(! pGPIOx) return STATUS_INVALID_PARAM;

	pGPIOx->ODR = (uint32_t) val;

	return STATUS_OK;
}

/*********************************************************************
 * @fn      		 			- GPIO_ToggleOutputPin
 *
 * @brief             		- This function toggles the output value of a given GPIO pin
 *
 * @param[in]       	- base address of the GPIO port peripheral
 * @param[in]        	- the pin number of the pin to toggle
 *
 * @return           		-  status indicating whether the command was successful or not
 *
 * @Note            		-  none
 **********************************************************************/
status_t GPIO_ToggleOutputPin(GPIO_reg_t *pGPIOx, uint8_t pinNumber)
{
	if(pinNumber > GPIO_PIN_15 || ! pGPIOx) return STATUS_INVALID_PARAM;

	pGPIOx->BSRR = ( pGPIOx->ODR & (1U << pinNumber) )
		                    ? (1U << (pinNumber + 16U))		// if HIGH then RESET
		                    :  (1U << pinNumber);						// if LOW then SET

	return STATUS_OK;
}

/*********************************************************************
 * @fn      		 			- GPIO_IRQConfig
 *
 * @brief             		- TODO
 *
 * @param[in]       	- TODO
 * @param[in]        	- TODO
 * @param[in]        	- TODO
 *
 * @return           		-  status indicating whether the command was successful or not
 *
 * @Note            		-  none
 **********************************************************************/
status_t GPIO_IRQConfig(uint8_t IRQNumber, uint8_t IRQPriority, uint8_t enable)
{

}

/*********************************************************************
 * @fn      		 			- GPIO_IRQHandling
 *
 * @brief             		- TODO
 *
 * @param[in]       	- TODO
 *
 * @return           		-  status indicating whether the command was successful or not
 *
 * @Note            		-  none
 **********************************************************************/
status_t GPIO_IRQHandling(uint8_t pinNumber)
{

}

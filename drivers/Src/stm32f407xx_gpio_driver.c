/*
 * stm32f407xx_gpio_driver.c
 *
 *  Created on: Jun 11, 2026
 *      Author: ernad
 */

#include "stm32f407xx_gpio_driver.h"

static void inline GPIO_write_field(uint32_t *reg, GPIO_pin_number_t pin, uint32_t width, uint32_t attr)
{
	uint32_t shift 	= width * pin;
	uint32_t mask = ((1U << width) - 1U) << shift;
	attr 					 &= ((1U << width) - 1U) ;

	*reg &= ~mask;

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
status_t GPIO_init(GPIO_handle_t *pGPIOHandle)				// TODO optimize by removing mask/shift sets/resets
{
	if(!pGPIOHandle) return STATUS_INVALID_PARAM;

	uint32_t pinNumber 	= pGPIOHandle->GPIO_PinConfig.pinNumber;
	if(pinNumber > GPIO_PIN_15) return STATUS_INVALID_PARAM;

	uint32_t shift = 0;
	uint32_t attr = 0;
	uint32_t mask = 0;

	// ======================== 1. Configure the mode of GPIO pin ========================
	if(pGPIOHandle->GPIO_PinConfig.mode <= GPIO_MODE_ANALOG)		// non-interrupt modes
	{
	    shift 			= 2U * pinNumber;
		attr 				= (uint32_t) (pGPIOHandle->GPIO_PinConfig.mode & 0x3U);
		mask 			= (0x3U << shift);

		// clear the current 2 mode bits
		pGPIOHandle->pGPIOx->MODER &= ~mask;

		// set the new mode bits
		pGPIOHandle->pGPIOx->MODER |= (attr << shift);
	}
	else	// interrupt modes
	{
			// TODO set interrupt modes
	}

	shift 	= 0;
	attr 		= 0;
	mask 	= 0;

	// ======================== 2. Configure the speed of GPIO pin ========================
	shift 			= 2U * pinNumber;
	attr 				= (uint32_t) ( pGPIOHandle->GPIO_PinConfig.speed & 0x3U);
	mask 			= (3U << shift);

	// clear the current 2 speed bits
	pGPIOHandle->pGPIOx->OSPEEDR &= ~mask;

	// set the new speed bits
	pGPIOHandle->pGPIOx->OSPEEDR |= (attr << shift);

	shift 	= 0;
	attr 		= 0;
	mask 	= 0;

	// 3. ======================== Configure pull-up/pull-down settings of GPIO pin ========================
	shift 			= 2U * pinNumber;
	attr				= (uint32_t) (pGPIOHandle->GPIO_PinConfig.puPdCtl & 0x3U);
	mask 			= (0x3U << shift);

	// clear the current 2 PU/PD bits
	pGPIOHandle->pGPIOx->PUPDR &= ~mask;

	// set the new PU/PD bits
	pGPIOHandle->pGPIOx->PUPDR |= (attr << shift);


	shift 	= 0;
	attr 		= 0;
	mask 	= 0;

	// 4. ======================== Configure the output type ========================
	if(pGPIOHandle->GPIO_PinConfig.mode == GPIO_MODE_OUT ||
		pGPIOHandle->GPIO_PinConfig.mode == GPIO_MODE_ALTFN)
	{
		shift 			=  pinNumber;
		attr 				= (uint32_t) (pGPIOHandle->GPIO_PinConfig.outType & 0x1U);
		mask 			= (0x1U << shift);

		// clear the current output-type bit
		pGPIOHandle->pGPIOx->OTYPER &= ~mask;

		// set the new output-type bit
		pGPIOHandle->pGPIOx->OTYPER |= (attr << shift);
	}

	shift 	= 0;
	attr 		= 0;
	mask 	= 0;

	// 4. ======================== Configure the alternate functionality ========================
	if(pGPIOHandle->GPIO_PinConfig.mode == GPIO_MODE_ALTFN)
	{
		uint8_t afrIdx	= (uint8_t) (pinNumber / 8U);
		shift 					= (uint32_t) (4U * (pinNumber % 8U));
		attr 						= (uint32_t) (pGPIOHandle->GPIO_PinConfig.altFuncMode & 0x0FU);
		mask 					= (0x0FU << shift);

		// clear the current alt-func bits
		pGPIOHandle->pGPIOx->AFR[afrIdx] &= ~mask;

		// set the new alt-func bits
		pGPIOHandle->pGPIOx->AFR[afrIdx] |= (attr << shift);
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
 * @return           		-  returns the read value from the input register
 *
 * @Note            		-  none
 **********************************************************************/
uint8_t GPIO_ReadPin(GPIO_reg_t *pGPIOx, uint8_t pinNumber)
{

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

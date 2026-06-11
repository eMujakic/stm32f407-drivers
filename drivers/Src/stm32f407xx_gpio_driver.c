/*
 * stm32f407xx_gpio_driver.c
 *
 *  Created on: Jun 11, 2026
 *      Author: ernad
 */

#include "stm32f407xx_gpio_driver.h"

/*********************************************************************
 * @fn      		 			- GPIO_init
 *
 * @brief             		- This function initializes a give GPIO pin according to the config settings in the handle
 *
 * @param[in]       	- pointer to a struct containing GPIO port pointer and config struct
 *
 * @return           		-  none
 *
 * @Note            		-  none
 **********************************************************************/
void GPIO_init(GPIO_handle_t *pGPIOHandle)
{

}

/*********************************************************************
 * @fn      		 			- GPIO_Reset
 *
 * @brief             		- This function resets a given GPIO port to default settings
 *
 * @param[in]       	- base address of the GPIO port peripheral
 *
 * @return           		-  none
 *
 * @Note            		-  none
 **********************************************************************/
void GPIO_Reset(GPIO_REG_T *pGPIOx)
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
 * @return           		-  none
 *
 * @Note            		-  none
 **********************************************************************/
void GPIO_PeriClkCtl(GPIO_REG_T *pGPIOx, uint8_t enable)
{

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
uint8_t GPIO_ReadPin(GPIO_REG_T *pGPIOx, uint8_t pinNumber)
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
uint16_t GPIO_ReadPort(GPIO_REG_T *pGPIOx)
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
 * @return           		-  none
 *
 * @Note            		-  none
 **********************************************************************/
void GPIO_WritePin(GPIO_REG_T *pGPIOx, uint8_t pinNumber, uint8_t val)
{

}

/*********************************************************************
 * @fn      		 			- GPIO_WritePort
 *
 * @brief             		- This function writes the entire output register of a given GPIO port
 *
 * @param[in]       	- base address of the GPIO port peripheral
 *
 * @return           		-  none
 *
 * @Note            		-  none
 **********************************************************************/
void GPIO_WritePort(GPIO_REG_T *pGPIOx, uint16_t val)
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
 * @return           		-  none
 *
 * @Note            		-  none
 **********************************************************************/
void GPIO_ToggleOutputPin(GPIO_REG_T *pGPIOx, uint8_t pinNumber)
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
 * @return           		-  none
 *
 * @Note            		-  none
 **********************************************************************/
void GPIO_IRQConfig(uint8_t IRQNumber, uint8_t IRQPriority, uint8_t enable)
{

}

/*********************************************************************
 * @fn      		 			- GPIO_IRQHandling
 *
 * @brief             		- TODO
 *
 * @param[in]       	- TODO
 *
 * @return           		-  none
 *
 * @Note            		-  none
 **********************************************************************/
void GPIO_IRQHandling(uint8_t pinNumber)
{

}

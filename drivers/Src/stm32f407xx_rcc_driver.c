/*
 * stm32f407xx_rcc_driver.c
 *
 *  Created on: Oct 29, 2026
 *      Author: ernad
 */

#include "stm32f407xx_rcc_driver.h"


/**************************************************************************
 * @fn		RCC_HzToMHz
 *
 * @brief	Returns the number of MHz in hz.
 *
 * @param[in] hz        Frequency in hz.
 *
 * @return	The number of MHz in hz, with any fractional MHz discarded.
 ***************************************************************************/
uint32_t RCC_HzToMHz(uint32_t hz)
{
    return hz / RCC_HZ_PER_MHZ;
}

/**************************************************************************
 * @fn		RCC_GetSYSCLKspeed
 *
 * @brief	Returns the speed of SYSCLK in Hz.
 *
 * @return	The SYSCLK speed in Hz.
 *
 * @note	This function does not support PLL as SYSCLK source. If PLL
 *          is selected, the HSI speed will be returned instead.
 ***************************************************************************/
uint32_t RCC_GetSYSCLKspeed(void)
{
    uint8_t clksrc = (RCC->CFGR >> RCC_CFGR_SWS0_BIT) & 0x3U;

    if (clksrc == 0) return RCC_HSI_HZ;
    if (clksrc == 1) return RCC_HSE_HZ;
    // if (clksrc == 2) return RCC_HSI_HZ; // PLL

    return RCC_HSI_HZ;
}

/**************************************************************************
 * @fn		RCC_GetHCLKSpeed
 *
 * @brief	Returns the speed of the SYSCLK after prescaling.
 *
 * @return	The SYSCLK speed after prescaling.
 ***************************************************************************/
uint32_t RCC_GetHCLKSpeed(void)
{
    uint32_t reg = RCC->CFGR;
    uint8_t val = (reg >> RCC_CFGR_HPRE_BIT) & (0xFU);

    uint16_t prescaler = 1U;
    uint32_t sysclk = RCC_GetSYSCLKspeed();

    if (val < 8U) return sysclk / prescaler;

    if (val == 8U)  prescaler = 2U;
    if (val == 9U)  prescaler = 4U;
    if (val == 10U) prescaler = 8U;
    if (val == 11U) prescaler = 16U;
    if (val == 12U) prescaler = 64U;
    if (val == 13U) prescaler = 128U;
    if (val == 14U) prescaler = 256U;
    if (val == 15U) prescaler = 512U;

    return sysclk / prescaler;
}

/**************************************************************************
 * @fn		RCC_GetAPB1Prescaler
 *
 * @brief	Gets the value of the APB1 clock prescaler.
 *
 * @return	The APB1 clock divisor (1, 2, 4, 8, 16).
 *
 * @note	This function assumes that all I2C peripherals are hanging
 *          off of the APB1 bus.
 ***************************************************************************/
 static uint8_t RCC_GetAPB1Prescaler(void)
 {
    uint32_t reg = RCC->CFGR;
    uint8_t val = (reg >> RCC_CFGR_PPRE1_BIT) & (0x7U);
    
    if (val == 4U) return 2U;
    if (val == 5U) return 4U;
    if (val == 6U) return 8U;
    if (val == 7U) return 16U;

    return 1U;
}

/**************************************************************************
 * @fn		RCC_GetPCLK1Speed
 *
 * @brief	Returns the speed of PCLK1.
 *
 * @return	PCLK speed in Hz.
 ***************************************************************************/
uint32_t RCC_GetPCLK1Speed(void)
{
    uint32_t hclk = RCC_GetHCLKSpeed();
    uint32_t prescaler = RCC_GetAPB1Prescaler();

    return hclk / prescaler;
}
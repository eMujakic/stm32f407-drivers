/*
 * stm32f407xx_i2c_driver.c
 *
 *  Created on: Oct 27, 2026
 *      Author: ernad
 */

#include "stm32f407xx_i2c_driver.h"

/**************************************************************************
 * @fn		I2C_IsValidInstance
 *
 * @brief	Used to determine whether arg is a valid I2C base addr.
 *
 * @param[in] pI2Cx		I2C peripheral base address.
 *
 * @return	Boolean indicating validity of passed I2C base addr.
 ***************************************************************************/
static inline uint8_t I2C_IsValidInstance(I2C_reg_t *pI2Cx)
{
    return 
    (
        (pI2Cx == I2C1) || 
        (pI2Cx == I2C2) || 
        (pI2Cx == I2C3)
    );
}

/**************************************************************************
 * @fn		I2C_PeriClkCtl
 *
 * @brief	Enables/Disables the peripheral clock for an I2C peripheral.
 *
 * @param[in] pI2Cx		Pointer to I2C's register base address.
 * @param[in] enable	Boolean saying if clk should be enabled or disabled.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_INVALID_PARAM).
 *
 * @note	This function is automatically called in I2C_Init(), so the user
 *          typically doesn't have to call this function explicitly.
 ***************************************************************************/
status_t I2C_PeriClkCtl(I2C_reg_t *pI2Cx, uint8_t enable)
{
    // Validate arguments
    if (!I2C_IsValidInstance(pI2Cx)) return STATUS_INVALID_PARAM;
    if (enable != ENABLE && enable != DISABLE) return STATUS_INVALID_PARAM;

    if (pI2Cx == I2C1)
    {
        if (enable == ENABLE)   I2C1_PCLK_EN();
        else                    I2C1_PCLK_DI();
    }

    else if (pI2Cx == I2C2)
    {
        if (enable == ENABLE)   I2C2_PCLK_EN();
        else                    I2C2_PCLK_DI();
    }

    else if (pI2Cx == I2C3)
    {
        if (enable == ENABLE)   I2C3_PCLK_EN();
        else                    I2C3_PCLK_DI();
    }
    else return STATUS_INVALID_PARAM;

    return STATUS_OK;
}

/**************************************************************************
 * @fn		I2C_ConfigInit
 *
 * @brief	Sets I2C_config_t struct to safe default values
 *
 * @details
 * Initializes the passed I2C_config struct with standard-mode speed,
 * 0x00U device-address, automatic ACKing, and ~33.3% FM-SCL duty cycle.
 *
 * @param[in] pConfig	Pointer to I2C_config_t struct to be initialized.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_INVALID_PARAM).
 *
 * @note	This function is primarily for QOL, so the user doesn't need to
 *          explicitly set every struct value before calling I2C_Init().
 ***************************************************************************/
 status_t I2C_ConfigInit(I2C_config_t *pConfig)
 {
    if (!pConfig)
        return STATUS_INVALID_PARAM;

    pConfig->I2C_SCLSpeed       = I2C_SCL_SPEED_SM;     // Standard-mode speed (400KHz)
    pConfig->I2C_DeviceAddr     = 0x00U;                // Own slave device addr
    pConfig->I2C_ACKControl     = I2C_ACK_EN;           // Enable automatic ACKing
    pConfig->I2C_FMDutyCycle    = I2C_FM_DUTY_2;        // ~33.3% duty cycle

    return STATUS_OK;
 }

/**************************************************************************
 * @fn		I2C_Init
 *
 * @brief	Configures an I2C pin according to I2C_config_t struct
 *
 * @details
 * TODO
 *
 * @param[in] pI2CHandle	I2C handle for the peripheral to be initialized.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_INVALID_PARAM).
 *
 * @note	Supports only 7-bit slave addressing.
 ***************************************************************************/
status_t I2C_Init(I2C_handle_t *pI2CHandle)
{
    // ================ Validate Inputs ================
    if (!pI2CHandle || !pI2CHandle->pI2Cx)
        return STATUS_INVALID_PARAM;

    if (!I2C_IsValidInstance(pI2CHandle->pI2Cx))
        return STATUS_INVALID_PARAM;

    // ================ Validate Configuration Settings ================
    I2C_config_t config = pI2CHandle->I2C_Config;

    if (config.I2C_SCLSpeed > I2C_SCL_SPEED_FM ||
        config.I2C_SCLSpeed < I2C_SCL_SPEED_SM)
        return STATUS_INVALID_PARAM;
    
    if (config.I2C_ACKControl > I2C_ACK_EN)
        return STATUS_INVALID_PARAM;

    if (config.I2C_FMDutyCycle > I2C_FM_DUTY_16_9)
        return STATUS_INVALID_PARAM;

    uint32_t tempreg = 0U;
    uint32_t pclk1 = RCC_GetPCLK1Speed();

    // ================ Enable Peripheral Clock ================
    I2C_PeriClkCtl(pI2CHandle->pI2Cx, ENABLE);

    // ================ Disable I2C Peripheral ================
    pI2CHandle->pI2Cx->CR1 &= ~(1U << I2C_CR1_PE_BIT);

    // ================ Configure APB-FREQ Field ================
    tempreg = RCC_HzToMHz(pclk1);
    pI2CHandle->pI2Cx->CR2 &= ~(0x3FU << I2C_CR2_FREQ_BIT);
    pI2CHandle->pI2Cx->CR2 |= (tempreg & 0x3FU);
    tempreg = 0; // reset tempreg

    // ================ Configure Device-Addr ================
    tempreg = (config.I2C_DeviceAddr & 0x7FU) << I2C_OAR1_ADD_BIT;
    pI2CHandle->pI2Cx->OAR1 &= ~(0x7FU << I2C_OAR1_ADD_BIT);
    pI2CHandle->pI2Cx->OAR1 |= tempreg;

    pI2CHandle->pI2Cx->OAR1 &= ~(1U << I2C_OAR1_ADDMODE_BIT); // fixed 7-bit addressing
    pI2CHandle->pI2Cx->OAR1 |= (1U << 14U); // 14th bit should always be asserted
    tempreg = 0;

    // ================ Configure Mode (standard/fast) & Duty Cycle ================
    pI2CHandle->pI2Cx->CCR &= ~(1U << I2C_CCR_FS_BIT);
    if (config.I2C_SCLSpeed > I2C_SCL_SPEED_SM)
    {
        pI2CHandle->pI2Cx->CCR |= (1U << I2C_CCR_FS_BIT); // assert FS bit
        pI2CHandle->pI2Cx->CCR &= ~(1U << I2C_CCR_DUTY_BIT); // clear DUTY bit
        pI2CHandle->pI2Cx->CCR |= (( (uint32_t) config.I2C_FMDutyCycle) << I2C_CCR_DUTY_BIT); // write DUTY bit
    }
        
    // ================ Calculate/Configure CCR ================
    uint32_t ccr = 0;
    uint8_t ccr_constant = 0;
    if (config.I2C_SCLSpeed > I2C_SCL_SPEED_SM) // fast mode
    {
        if (config.I2C_FMDutyCycle == I2C_FM_DUTY_2)
            ccr_constant = 3;
        else if (config.I2C_FMDutyCycle == I2C_FM_DUTY_16_9)
            ccr_constant = 25;
    }
    else // standard mode
    {
        ccr_constant = 2; // 50% duty cycle in SM
    }

    // round CCR upwards
    ccr = (pclk1 + (ccr_constant * config.I2C_SCLSpeed) - 1U) / (ccr_constant * config.I2C_SCLSpeed);
    tempreg = (ccr & 0xFFFU) << I2C_CCR_CCR_BIT;

    pI2CHandle->pI2Cx->CCR &= ~(0xFFFU << I2C_CCR_CCR_BIT);
    pI2CHandle->pI2Cx->CCR |= tempreg;
    tempreg = 0;

    // ================ Configure Rise Time ================
    // TODO

    // ================ Enable I2C Peripheral ================
    pI2CHandle->pI2Cx->CR1 |= (1U << I2C_CR1_PE_BIT);

    // ================ Configure ACK-Control ================
    pI2CHandle->pI2Cx->CR1 &= ~(1U << I2C_CR1_ACK_BIT); // clear current ACK-bit
    pI2CHandle->pI2Cx->CR1 |= ((uint32_t) config.I2C_ACKControl << I2C_CR1_ACK_BIT); // write new ACK bit

    return STATUS_OK;
}

/**************************************************************************
 * @fn		I2C_Reset
 *
 * @brief	Resets a I2C peripheral to default register values
 *
 * @param[in] pI2Cx	    Pointer to I2C's base address.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_ERROR / STATUS_INVALID_PARAM).
 ***************************************************************************/
status_t I2C_Reset(I2C_reg_t *pI2Cx)
{
    if (!I2C_IsValidInstance(pI2Cx)) return STATUS_INVALID_PARAM;

    if      (pI2Cx == I2C1) I2C1_RESET();
    else if (pI2Cx == I2C2) I2C2_RESET();
    else if (pI2Cx == I2C3) I2C3_RESET();
    else                    return STATUS_INVALID_PARAM;
        
    return STATUS_OK;
}

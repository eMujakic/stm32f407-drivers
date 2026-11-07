/*
 * stm32f407xx_i2c_driver.c
 *
 *  Created on: Oct 27, 2026
 *      Author: ernad
 */

#include "stm32f407xx_i2c_driver.h"

// ********************************************************************
// ********************* PRIVATE HELPER FUNCTIONS *********************
// ********************************************************************

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
 * @fn		I2C_ClearADDRFlag
 *
 * @brief	Clears the ADDR flag by reading SR1, then reading SR2.
 *
 * @param[in] pI2Cx	    Pointer to I2C's base address.
 *
 * @return	None.
 *
 * @note	The ADDR flag must be cleared to release the SCL hold
 *          after the address phase, allowing data to be transmitted.
 ***************************************************************************/
static void I2C_ClearADDRFlag(I2C_reg_t *pI2Cx)
{
    if (!pI2Cx || !I2C_IsValidInstance(pI2Cx)) return;

    (void) pI2Cx->SR1;
    (void) pI2Cx->SR2;
}

/**************************************************************************
 * @fn		I2C_ExecuteAddrPhase
 *
 * @brief	Formats and sends 7-bit address and read/write bit.
 *
 * @param[in] pI2Cx		Pointer to I2C's base address.
 * @param[in] slaveAddr 7-bit address of slave target.
 * @param[in] rw		Read/Write bit.
 *
 * @return	(STATUS_OK): if the address byte was written to DR.
 *			(STATUS_INVALID_PARAM): if an input was invalid.
 *
 * @note	Only supports 7-bit slave addressing.
 ***************************************************************************/
static status_t I2C_ExecuteAddrPhase(I2C_reg_t *pI2Cx, uint8_t slaveAddr, I2C_rw_t rw)
{
    // validate inputs
    if (!pI2Cx || !I2C_IsValidInstance(pI2Cx)) return STATUS_INVALID_PARAM;
    if (rw != I2C_READ &&
        rw != I2C_WRITE) return STATUS_INVALID_PARAM;

    // ensure slave-addr is only 7-bits
    if (slaveAddr > 0x7FU) return STATUS_INVALID_PARAM;

    uint8_t addrByte = 0U;
    addrByte |= (slaveAddr << 1U) | (rw & 0x1U);
    pI2Cx->DR = addrByte; // send formatted addr byte

    return STATUS_OK;
}

/**************************************************************************
 * @fn		I2C_RequestStartCondition
 *
 * @brief	Requests a start-condition by asserting start-bit in CR1.
 *
 * @param[in] pI2Cx	    Pointer to I2C's base address.
 *
 * @return	None.
 *
 * @note    Wait for SB to be set before sending address byte.
 ***************************************************************************/
static void I2C_RequestStartCondition(I2C_reg_t *pI2Cx)
{
    // validate base-address pointer
    if(!pI2Cx || !I2C_IsValidInstance(pI2Cx)) return;

    pI2Cx->CR1 |= (1U << I2C_CR1_START_BIT);
}

/**************************************************************************
 * @fn		I2C_RequestStopCondition
 *
 * @brief	Requests a stop-condition by asserting stop-bit in CR1.
 *
 * @param[in] pI2Cx	    Pointer to I2C's base address.
 *
 * @return	None.
 ***************************************************************************/
static void I2C_RequestStopCondition(I2C_reg_t *pI2Cx)
{
    // validate base-address pointer
    if(!pI2Cx || !I2C_IsValidInstance(pI2Cx)) return;

    pI2Cx->CR1 |= (1U << I2C_CR1_STOP_BIT);
}

/**************************************************************************
 * @fn		I2C_MasterErrHandling
 *
 * @brief	Basic error handling if the passed peripheral is an I2C master.
 *
 * @details
 * Clears ARLO, BERR, PECERR, OVR, and AF errors if detected, and requests
 * a stop condition if appropriate.
 *
 * @param[in] pI2Cx	    Pointer to I2C's base address.
 *
 * @return	None.
 ***************************************************************************/
static void I2C_MasterErrHandling(I2C_reg_t *pI2Cx)
{
    // validate I2C pointer
    if (!pI2Cx || !I2C_IsValidInstance(pI2Cx))
        return;
    
    // arbitration error
    uint8_t arloErr = (pI2Cx->SR1 >> I2C_SR1_ARLO_BIT) & 0x1U;
    if (arloErr)
    {
        // clear ARLO flag
        pI2Cx->SR1 &= ~(1U << I2C_SR1_ARLO_BIT);
    }
    // bus error
    if (pI2Cx->SR1 & (1U << I2C_SR1_BERR_BIT)) 
    {
        // clear BERR flag
        pI2Cx->SR1 &= ~(1U << I2C_SR1_BERR_BIT); 
    }
    // PEC mismatch
    if (pI2Cx->SR1 & (1U << I2C_SR1_PECERR_BIT))
    {
        // clear PECERR flag
        pI2Cx->SR1 &= ~(1U << I2C_SR1_PECERR_BIT);
    }
    // Over/Under-run
    if (pI2Cx->SR1 & (1U << I2C_SR1_OVR_BIT)) 
    {
        // clear OVR flag
        pI2Cx->SR1 &= ~(1U << I2C_SR1_OVR_BIT);
    }
    
    // request stop condition if still master, no ARLO error,
    // and no pending stop already.
    if (I2C_GetFlag(pI2Cx, I2C_MSL_FLAG) && (arloErr == 0U) &&
        ((pI2Cx->CR1 >> I2C_CR1_STOP_BIT) & 0x1U) == 0U)
        I2C_RequestStopCondition(pI2Cx);

    if ( pI2Cx->SR1 & (1U << I2C_SR1_AF_BIT) ) // NACK
        pI2Cx->SR1 &= ~(1U << I2C_SR1_AF_BIT); // clear AF flag AFTER requesting stop
}

/**************************************************************************
 * @fn		I2C_ACKControl
 *
 * @brief	Enables/disables ACK bit for a given I2C peripheral.
 *
 * @param[in] pI2Cx	    Pointer to I2C's base address.
 * @param[in] EnorDi	Enable/Disable
 *
 * @return	STATUS_OK: The ACK was successfully set/reset.
 *          STATUS_INVALID_PARAM: Something wrong with argument(s) passed.
 *          STATUS_ERROR: The passed peripheral was disabled (PE == 0).
 ***************************************************************************/
static status_t I2C_ACKControl(I2C_reg_t *pI2Cx, uint8_t EnorDi)
{
    // Validate inputs
    if (!pI2Cx || !I2C_IsValidInstance(pI2Cx))
        return STATUS_INVALID_PARAM;

    if (EnorDi != ENABLE &&
        EnorDi != DISABLE)
        return STATUS_INVALID_PARAM;

    // Validate peripheral enabled
    if ( (pI2Cx->CR1 & (1U << I2C_CR1_PE_BIT)) == 0U )
        return STATUS_ERROR;
    
    // ================ Configure ACK-Control ================
    if (EnorDi == ENABLE)
    {
        pI2Cx->CR1 |= (1U << I2C_CR1_ACK_BIT); // set ACK-bit
    }
    else
    {
        pI2Cx->CR1 &= ~(1U << I2C_CR1_ACK_BIT); // clear ACK-bit
    }

    return STATUS_OK;
}

/**************************************************************************
 * @fn		I2C_RestoreACK
 *
 * @brief	Restores ACK bit in CR1 register to desired configuration.
 *
 * @param[in] pI2CHandle    Handle for an I2C peripheral.
 *
 * @return	STATUS_OK: The ACK was successfully restored.
 *          STATUS_INVALID_PARAM: Something wrong with argument(s) passed.
 *          STATUS_TIMEOUT: Timed-out waiting for STOP bit to clear.
 *
 * @note	This API can timeout waiting for the STOP bit in CR1 to clear.
 *          It is not correct to try to set/clear the ACK bit while a 
 *          STOP is still pending.
 ***************************************************************************/
static status_t I2C_RestoreACK(I2C_handle_t *pI2CHandle)
{
    // Validate inputs
    if (!pI2CHandle || !pI2CHandle->pI2Cx ||
        !I2C_IsValidInstance(pI2CHandle->pI2Cx))
        return STATUS_INVALID_PARAM;

    // Wait for STOP to be cleared
    uint32_t timeout = I2C_TIMEOUT;
    while ( (pI2CHandle->pI2Cx->CR1 & (1U << I2C_CR1_STOP_BIT)) != 0U )
    {
        if (timeout == 0U)
            return STATUS_TIMEOUT;
        timeout--;
    }

    // Restore Initial ACK configuration
    if (pI2CHandle->I2C_Config.I2C_ACKControl == I2C_ACK_EN)
    {
        pI2CHandle->pI2Cx->CR1 |= (1U << I2C_CR1_ACK_BIT);
    }
    else
    {
        pI2CHandle->pI2Cx->CR1 &= ~(1U << I2C_CR1_ACK_BIT);
    }

    return STATUS_OK;
}

/**************************************************************************
 * @fn		I2C_InterruptControl
 *
 * @brief	Enables/disables all interrupts for an I2C peripheral.
 *
 * @param[in] pI2Cx	    Pointer to I2C's base address.
 *
 * @return	None.
 ***************************************************************************/
static void I2C_InterruptControl(I2C_reg_t *pI2Cx, uint8_t EnorDi)
{
    if(!pI2Cx || !I2C_IsValidInstance(pI2Cx))
        return;

    if (EnorDi == DISABLE)
    {
    pI2Cx->CR2  &= 
        ~(  (1U << I2C_CR2_ITERREN_BIT) |
            (1U << I2C_CR2_ITEVTEN_BIT) |
            (1U << I2C_CR2_ITBUFEN_BIT));
    }
    
    else if (EnorDi == ENABLE)
    {
        pI2Cx->CR2  |= 
        (   (1U << I2C_CR2_ITERREN_BIT) |
            (1U << I2C_CR2_ITEVTEN_BIT) |
            (1U << I2C_CR2_ITBUFEN_BIT));
    }
}

// ***************************************************************
// ********************* API IMPLEMENTATIONS *********************
// ***************************************************************

/**************************************************************************
 * @fn		I2C_SoftwareReset
 *
 * @brief	Triggers the software reset for a given I2C peripheral.
 *
 * @param[in] pI2Cx	    Pointer to I2C's base address.
 *
 * @return	STATUS_OK: The peripheral was successfully reset.
 *          STATUS_INVALID_PARAM: Something wrong with passed arguments.
 *
 * @note	This calls resets all configurations applied to the I2C
 *          peripheral. The user will likely need to call I2C_Init()
 *          again.
 ***************************************************************************/
status_t I2C_SoftwareReset(I2C_reg_t *pI2Cx)
{
    if(!pI2Cx || !I2C_IsValidInstance(pI2Cx))
        return STATUS_INVALID_PARAM;

    // set SWRST bit
    pI2Cx->CR1 |= (1U << I2C_CR1_SWRST_BIT);
    // clear SWRST bit
    pI2Cx->CR1 &= ~(1U << I2C_CR1_SWRST_BIT);

    return STATUS_OK;
}

/**************************************************************************
 * @fn		I2C_GetFlag
 *
 * @brief	Returns the value of the requested flag in SR1/SR2.
 *
 * @param[in] pI2Cx		Pointer to I2C's base address.
 * @param[in] flag	    Enum value of desired flag to read.
 *
 * @return	0U/1U for single-bit flags.
 *          8-bit PEC value for I2C_PEC_FLAG.
 *          0U for invalid arguments.
 *
 * @note	Returns 0U when invalid argument is passed.
 * @note    Reading SR registers can cause hardware side-effects,
 *          for example, clearing ADDR.
 ***************************************************************************/
uint8_t  I2C_GetFlag(I2C_reg_t *pI2Cx, I2C_flag_t flag)
{
    // validate base-address pointer
    if(!pI2Cx || !I2C_IsValidInstance(pI2Cx)) return 0U;

    switch(flag)
    {
        case I2C_SB_FLAG:
            return ( (pI2Cx->SR1 >> I2C_SR1_SB_BIT) & 0x1U );
        
        case I2C_ADDR_FLAG:
            return ( (pI2Cx->SR1 >> I2C_SR1_ADDR_BIT) & 0x1U );
        
        case I2C_BTF_FLAG:
            return ( (pI2Cx->SR1 >> I2C_SR1_BTF_BIT) & 0x1U );
        
        case I2C_ADD10_FLAG:
            return ( (pI2Cx->SR1 >> I2C_SR1_ADD10_BIT) & 0x1U );
        
        case I2C_STOPF_FLAG:
            return ( (pI2Cx->SR1 >> I2C_SR1_STOPF_BIT) & 0x1U );
        
        case I2C_RxNE_FLAG:
            return ( (pI2Cx->SR1 >> I2C_SR1_RxNE_BIT) & 0x1U );
        
        case I2C_TxE_FLAG:
            return ( (pI2Cx->SR1 >> I2C_SR1_TxE_BIT) & 0x1U );
        
        case I2C_BERR_FLAG:
            return ( (pI2Cx->SR1 >> I2C_SR1_BERR_BIT) & 0x1U );
        
        case I2C_ARLO_FLAG:
            return ( (pI2Cx->SR1 >> I2C_SR1_ARLO_BIT) & 0x1U );
        
        case I2C_AF_FLAG:
            return ( (pI2Cx->SR1 >> I2C_SR1_AF_BIT) & 0x1U );
        
        case I2C_OVR_FLAG:
            return ( (pI2Cx->SR1 >> I2C_SR1_OVR_BIT) & 0x1U );
        
        case I2C_PECERR_FLAG:
            return ( (pI2Cx->SR1 >> I2C_SR1_PECERR_BIT) & 0x1U );
        
        case I2C_TIMEOUT_FLAG:
            return ( (pI2Cx->SR1 >> I2C_SR1_TIMEOUT_BIT) & 0x1U );
        
        case I2C_SMBALERT_FLAG:
            return ( (pI2Cx->SR1 >> I2C_SR1_SMBALERT_BIT) & 0x1U );
        
        case I2C_MSL_FLAG:
            return ( (pI2Cx->SR2 >> I2C_SR2_MSL_BIT) & 0x1U );
        
        case I2C_BUSY_FLAG:
            return ( (pI2Cx->SR2 >> I2C_SR2_BUSY_BIT) & 0x1U );
        
        case I2C_TRA_FLAG:
            return ( (pI2Cx->SR2 >> I2C_SR2_TRA_BIT) & 0x1U );
        
        case I2C_GENCALL_FLAG:
            return ( (pI2Cx->SR2 >> I2C_SR2_GENCALL_BIT) & 0x1U );
        
        case I2C_SMBDEFAULT_FLAG:
            return ( (pI2Cx->SR2 >> I2C_SR2_SMBDEFAULT_BIT) & 0x1U );
        
        case I2C_SMBHOST_FLAG:
            return ( (pI2Cx->SR2 >> I2C_SR2_SMBHOST_BIT) & 0x1U );
        
        case I2C_DUALF_FLAG:
            return ( (pI2Cx->SR2 >> I2C_SR2_DUALF_BIT) & 0x1U );
        
        case I2C_PEC_FLAG:
            return ( (pI2Cx->SR2 >> I2C_SR2_PEC_BIT) & 0xFFU );
        
        default:
            return 0U;
    }
}

/**************************************************************************
 * @fn		I2C_WaitForFlag
 *
 * @brief	Spins until flag==val, software timeout is triggered, or an
 *          error flag (ARLO, BERR, AF, OVR, PECERR) is asserted.
 *
 * @param[in] pI2Cx		Pointer to I2C's base address.
 * @param[in] flag		Single-bit flag to poll.
 * @param[in] val		Desired value for flag.
 *
 * @return	STATUS_OK: flag == val before software timeout.
 *          STATUS_TIMEOUT: Software timeout triggered.
 *			STATUS_INVALID_PARAM: Invalid input.
 *          STATUS_ERROR: An error was detected.
 *
 * @note	Does not handle waiting for a specific value on 
 *          any SR2 flags to avoid accidental clears of ADDR.
 *
 * @note    Error flags are checked according to the order in 
 *          the brief. i.e. (ARLO, BERR, AF, OVR, PECERR).
 ***************************************************************************/
status_t I2C_WaitForFlag(I2C_reg_t *pI2Cx, I2C_flag_t flag, uint8_t val)
{
    // validate args
    if (!pI2Cx || !I2C_IsValidInstance(pI2Cx) ||
        flag < I2C_SB_FLAG || flag > I2C_SMBALERT_FLAG ||
        val > 1U)
    {
        return STATUS_INVALID_PARAM;
    }
        
    uint32_t timeout = I2C_TIMEOUT;
    for(;;)
    {
        // check for errors
        if (I2C_GetFlag(pI2Cx, I2C_ARLO_FLAG))
            return STATUS_ERROR;

        if (I2C_GetFlag(pI2Cx, I2C_BERR_FLAG))
            return STATUS_ERROR;

        if (I2C_GetFlag(pI2Cx, I2C_AF_FLAG))
            return STATUS_NACK;

        if (I2C_GetFlag(pI2Cx, I2C_OVR_FLAG))
            return STATUS_ERROR;

        if (I2C_GetFlag(pI2Cx, I2C_PECERR_FLAG))
            return STATUS_ERROR;

        // check desired flag
        if (I2C_GetFlag(pI2Cx, flag) == val)
            return STATUS_OK;

        // check timeout
        if (timeout == 0U) return STATUS_TIMEOUT;
        timeout--;
    }

    return STATUS_OK;
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

    pConfig->I2C_SCLSpeed       = I2C_SCL_SPEED_SM;     // Standard-mode speed (100KHz)
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

    if (config.I2C_DeviceAddr > 0x7FU)
        return STATUS_INVALID_PARAM;

    uint32_t tempreg = 0U;
    uint64_t pclk1 = RCC_GetPCLK1Speed();

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
    if (config.I2C_SCLSpeed > I2C_SCL_SPEED_SM) // fast mode
    {
        tempreg = ((pclk1 * I2C_TRISE_MAX_FM) / RCC_HZ_PER_GHZ) + 1;
    }
    else // standard mode
    {
        tempreg = (pclk1 / RCC_HZ_PER_MHZ) + 1;
    }
    pI2CHandle->pI2Cx->TRISE &= ~(0x3F);
    pI2CHandle->pI2Cx->TRISE |= (tempreg & 0x3F);
    tempreg = 0;

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
    else return STATUS_INVALID_PARAM;
        
    return STATUS_OK;
}

/**************************************************************************
 * @fn		I2C_MasterSendData
 *
 * @brief	Sends data as master using I2C protocol.
 *
 *
 * @param[in] pI2CHandle    Handle for a I2C peripheral.
 * @param[in] pTxBuffer		Pointer to data-buffer to be transfered.
 * @param[in] len		    Number of bytes to transfer.
 * @param[in] slaveAddr		Target slave address to transfer to.
 * @param[in] SR		    Boolean indicating whether to send stop-
 *                          condition (I2C_SR_DI) or not (I2C_SR_EN).
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_ERROR / STATUS_INVALID_PARAM).
 *
 * @note	If this API returns STATUS_TIMEOUT, that means an unrecoverable
 *          error occured and the peripheral had to be reset and
 *          reinitialized.
 ***************************************************************************/
status_t I2C_MasterSendData(I2C_handle_t *pI2CHandle, uint8_t *pTxBuffer,
                            uint32_t len, uint8_t slaveAddr, uint8_t SR)
{
    // ========== Validate Inputs ==========
    if (!pI2CHandle || !pI2CHandle->pI2Cx)
        return STATUS_INVALID_PARAM;

    if (!I2C_IsValidInstance(pI2CHandle->pI2Cx))
        return STATUS_INVALID_PARAM;

    if (!pTxBuffer)
        return STATUS_INVALID_PARAM;

    if (slaveAddr > 0x7FU)
        return STATUS_INVALID_PARAM;

    if (len < 1U)
        return STATUS_INVALID_PARAM;

    // ========= Request Start Condition =========
    I2C_RequestStartCondition(pI2CHandle->pI2Cx);

    // ========= Confirm Start Condition =========
    // wait for SB flag to be set
    status_t status = I2C_WaitForFlag(pI2CHandle->pI2Cx, I2C_SB_FLAG, SET);
    if (status != STATUS_OK)
    {
        if (status == STATUS_TIMEOUT)
        {
            I2C_SoftwareReset(pI2CHandle->pI2Cx);
            I2C_Init(pI2CHandle);
        }
        else
        {
            I2C_MasterErrHandling(pI2CHandle->pI2Cx);
        }
        
        return status;
    }
        
    // ========= Send Address and R/W Bit =========
    I2C_ExecuteAddrPhase(pI2CHandle->pI2Cx, slaveAddr, I2C_WRITE); // writes DR

    // ========= Confirm Address Phase Completion =========
    status = I2C_WaitForFlag(pI2CHandle->pI2Cx, I2C_ADDR_FLAG, SET);
    if (status != STATUS_OK)
    {
        I2C_MasterErrHandling(pI2CHandle->pI2Cx);
        return status;
    }

    // ========= Clear ADDR =========
    I2C_ClearADDRFlag(pI2CHandle->pI2Cx);

    // ========= Send Data =========
    while(len > 0)
    {
        status = I2C_WaitForFlag(pI2CHandle->pI2Cx, I2C_TxE_FLAG, SET); // wait until Tx buffer is empty
        if (status != STATUS_OK)
            {
                I2C_MasterErrHandling(pI2CHandle->pI2Cx);
                return status;
            }
        
        pI2CHandle->pI2Cx->DR = *pTxBuffer;
        pTxBuffer++;
        len--;
    }

    // ========= Close Transmission =========
    status = I2C_WaitForFlag(pI2CHandle->pI2Cx, I2C_BTF_FLAG, SET); // wait until byte-transfer complete
    if (status != STATUS_OK)
    {
        I2C_MasterErrHandling(pI2CHandle->pI2Cx);
        return status;
    }

    // ========= Request Stop Condition =========
    if (SR == I2C_SR_DI)
        I2C_RequestStopCondition(pI2CHandle->pI2Cx);

    return STATUS_OK;
}

/**************************************************************************
 * @fn		I2C_MasterReceiveData
 *
 * @brief	Receives data from slave as an I2C master.
 *
 * @details
 * This API receives data from an I2C-slave, while acting as an I2C-
 * master. It correctly separates the logic for sending 1-byte, 2-
 * bytes, and >2 bytes according to the reference manual.
 *
 * @param[in] pI2CHandle    Pointer to handle for an I2C peripheral.
 * @param[in] pRxBuffer		Pointer to Rx buffer.
 * @param[in] len		    Length of bytes to receive.
 * @param[in] slaveAddr		Address of slave to receive data from.
 * @param[in] SR		    Boolean indicating whether to send stop-
 *                          condition (I2C_SR_DI) or not (I2C_SR_EN).
 *
 * @return	STATUS_OK: Communication was performed sucessfully.
 *          STATUS_INVALID_PARAM: An invalid argument was passed to the API.
 *          STATUS_NACK: The slave did not ACK.
 *          STATUS_ERROR: An error occured that did not require a reset.
 *          STATUS_TIMEOUT: A critical error occured and the peripheral
 *                          had to be reset and reinitialized.
 *          STATUS_BUSY: A start/stop was pending, or the bus is occupied.
 *
 * @note	This API resets and reinitializes the peripheral whenever a
 *          STATUS_TIMEOUT is returned.
 ***************************************************************************/
status_t I2C_MasterReceiveData(I2C_handle_t *pI2CHandle,
    uint8_t *pRxBuffer, uint32_t len, uint8_t slaveAddr, uint8_t SR)
{
    // ========== Validate Inputs ==========
    if (!pI2CHandle || !pI2CHandle->pI2Cx)
        return STATUS_INVALID_PARAM;

    if (!I2C_IsValidInstance(pI2CHandle->pI2Cx))
        return STATUS_INVALID_PARAM;

    if (!pRxBuffer)
        return STATUS_INVALID_PARAM;

    if (slaveAddr > 0x7FU)
        return STATUS_INVALID_PARAM;

    if (len < 1U)
        return STATUS_INVALID_PARAM;

    // ========== Validate Peripheral State ==========
    // if peripheral is disabled
    if ( (pI2CHandle->pI2Cx->CR1 & (1U << I2C_CR1_PE_BIT)) == 0U )
        return STATUS_ERROR;

    // if start/stop is already pending
    if ( (pI2CHandle->pI2Cx->CR1 & ((1U << I2C_CR1_STOP_BIT) | (1U << I2C_CR1_START_BIT))) )
        return STATUS_BUSY;

    // ========== Enable ACKing ==========
    pI2CHandle->pI2Cx->CR1 |= (1U << I2C_CR1_ACK_BIT);

    // ========== Clear POS ==========
    pI2CHandle->pI2Cx->CR1 &= ~(1U << I2C_CR1_POS_BIT);


    // initial ACK configuration must be restored after this point.


    // ========= Request Start Condition =========
    I2C_RequestStartCondition(pI2CHandle->pI2Cx);

    // ========= Confirm Start Condition =========
    // wait for SB flag to be set
    status_t status = I2C_WaitForFlag(pI2CHandle->pI2Cx, I2C_SB_FLAG, SET);
    if (status != STATUS_OK)
    {
        if (status == STATUS_TIMEOUT)
        {
            I2C_SoftwareReset(pI2CHandle->pI2Cx);
            I2C_Init(pI2CHandle);
        }
        else
        {
            I2C_MasterErrHandling(pI2CHandle->pI2Cx);
            I2C_RestoreACK(pI2CHandle);
        }
        
        return status;
    }


    // acquired ownership of bus at this point
        

    // ========= Send Address and R/W Bit =========
    I2C_ExecuteAddrPhase(pI2CHandle->pI2Cx, slaveAddr, I2C_READ); // writes DR

    // ========= Confirm Address Phase Completion =========
    status = I2C_WaitForFlag(pI2CHandle->pI2Cx, I2C_ADDR_FLAG, SET);
    if (status != STATUS_OK)
    {
        I2C_MasterErrHandling(pI2CHandle->pI2Cx);
        if (I2C_RestoreACK(pI2CHandle) == STATUS_TIMEOUT)
        {
            I2C_SoftwareReset(pI2CHandle->pI2Cx);
            I2C_Init(pI2CHandle);
            return STATUS_TIMEOUT;
        }
        return status;
    }

    // ================== Receive Data ==================
    if (len == 1U)
    {
        // Disable ACKing
        I2C_ACKControl(pI2CHandle->pI2Cx, DISABLE);

        // Clear ADDR flag
        I2C_ClearADDRFlag(pI2CHandle->pI2Cx);

        // Request stop
        if (SR == I2C_SR_DI)
            I2C_RequestStopCondition(pI2CHandle->pI2Cx);

        // Wait until data-ready
        status = I2C_WaitForFlag(pI2CHandle->pI2Cx, I2C_RxNE_FLAG, SET);
        if (status != STATUS_OK)
        {
            I2C_MasterErrHandling(pI2CHandle->pI2Cx);
            if (I2C_RestoreACK(pI2CHandle) == STATUS_TIMEOUT)
            {
                I2C_SoftwareReset(pI2CHandle->pI2Cx);
                I2C_Init(pI2CHandle);
                return STATUS_TIMEOUT;
            }
            
            return status;
        }

        // Read data into buffer
        *pRxBuffer = pI2CHandle->pI2Cx->DR;
    }
    else if (len == 2U)
    {
        // Assert POS
        pI2CHandle->pI2Cx->CR1 |= (1U << I2C_CR1_POS_BIT);

        // Disable ACKing
        I2C_ACKControl(pI2CHandle->pI2Cx, DISABLE);

        // Clear ADDR flag
        I2C_ClearADDRFlag(pI2CHandle->pI2Cx);

        // Wait for BTF
        status = I2C_WaitForFlag(pI2CHandle->pI2Cx, I2C_BTF_FLAG, SET);
        if (status != STATUS_OK)
        {
            I2C_MasterErrHandling(pI2CHandle->pI2Cx);
            if (I2C_RestoreACK(pI2CHandle) == STATUS_TIMEOUT)
            {
                I2C_SoftwareReset(pI2CHandle->pI2Cx);
                I2C_Init(pI2CHandle);
                return STATUS_TIMEOUT;
            }
            return status;
        }

        // Request stop
        if (SR == I2C_SR_DI)
            I2C_RequestStopCondition(pI2CHandle->pI2Cx);

        // Read both bytes
        *pRxBuffer = pI2CHandle->pI2Cx->DR;

        pRxBuffer++;

        *pRxBuffer = pI2CHandle->pI2Cx->DR;
    }
    else // len > 2
    {
        // Clear ADDR flag
        I2C_ClearADDRFlag(pI2CHandle->pI2Cx);

        for (uint32_t i = len; i>0; i--)
        {
            // Wait until data-ready
            status = I2C_WaitForFlag(pI2CHandle->pI2Cx, I2C_RxNE_FLAG, SET);
            if (status != STATUS_OK)
            {
                I2C_MasterErrHandling(pI2CHandle->pI2Cx);
                if (I2C_RestoreACK(pI2CHandle) == STATUS_TIMEOUT)
                {
                    I2C_SoftwareReset(pI2CHandle->pI2Cx);
                    I2C_Init(pI2CHandle);
                    return STATUS_TIMEOUT;
                }
                return status;
            }

            if (i == 3U) // if only 3-bytes remaining
            {
                // Wait for BTF
                status = I2C_WaitForFlag(pI2CHandle->pI2Cx, I2C_BTF_FLAG, SET);
                if (status != STATUS_OK)
                {
                    I2C_MasterErrHandling(pI2CHandle->pI2Cx);
                    if (I2C_RestoreACK(pI2CHandle) == STATUS_TIMEOUT)
                    {
                        I2C_SoftwareReset(pI2CHandle->pI2Cx);
                        I2C_Init(pI2CHandle);
                        return STATUS_TIMEOUT;
                    }
                    return status;
                }
                
                // Disable ACKing
                I2C_ACKControl(pI2CHandle->pI2Cx, DISABLE);

                // Read data into buffer
                *pRxBuffer = pI2CHandle->pI2Cx->DR;
                pRxBuffer++;

                // Wait for BTF
                status = I2C_WaitForFlag(pI2CHandle->pI2Cx, I2C_BTF_FLAG, SET);
                if (status != STATUS_OK)
                {
                    I2C_MasterErrHandling(pI2CHandle->pI2Cx);
                    if (I2C_RestoreACK(pI2CHandle) == STATUS_TIMEOUT)
                    {
                        I2C_SoftwareReset(pI2CHandle->pI2Cx);
                        I2C_Init(pI2CHandle);
                        return STATUS_TIMEOUT;
                    }
                    return status;
                }

                // Request stop
                if (SR == I2C_SR_DI)
                    I2C_RequestStopCondition(pI2CHandle->pI2Cx);

                // Read final two bytes
                *pRxBuffer = pI2CHandle->pI2Cx->DR;

                pRxBuffer++;

                *pRxBuffer = pI2CHandle->pI2Cx->DR;

                break;
            }
            
            // Read data into buffer
            *pRxBuffer = pI2CHandle->pI2Cx->DR;
            pRxBuffer++;
        }
    }
    
    if (I2C_RestoreACK(pI2CHandle) == STATUS_TIMEOUT)
    {
        I2C_SoftwareReset(pI2CHandle->pI2Cx);
        I2C_Init(pI2CHandle);
        return STATUS_TIMEOUT;
    }

    return STATUS_OK;
}

// ***************************************************************
// ******************* INTERRUPT EVENT HELPERS *******************
// ***************************************************************

/**************************************************************************
 * @fn		I2C_CloseTransferMaster
 *
 * @brief	Disables interrupts and cleans handle members.
 *
 * @details
 * Disables all interrupts, cleans interrupt state in the passed handle,
 * and clears POS for receiving masters.
 *
 * @param[in] pI2CHandle    Pointer to handle for I2C peripheral.
 *
 * @return	None.
 ***************************************************************************/
static void I2C_CloseTransferMaster(I2C_handle_t *pI2CHandle)
{
    if (!pI2CHandle || !pI2CHandle->pI2Cx) return;

    // disable further interrupts
    I2C_InterruptControl(pI2CHandle->pI2Cx, DISABLE);

    if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
    {
        pI2CHandle->pI2Cx->CR1 &= ~(1U << I2C_CR1_POS_BIT);
    }
            
    // clean handle state
    pI2CHandle->TxRxState       = I2C_READY;
    pI2CHandle->targetAddr      = 0x00U;
    pI2CHandle->remaining       = 0U;
    pI2CHandle->transferSize    = 0U;
    pI2CHandle->pBuffer         = NULL;

    // I2C_RestoreACK(pI2CHandle); // TODO: this busy waits, can't be used in interrupt context
}

/**************************************************************************
 * @fn		I2C_CloseTransferSlave
 *
 * @brief	Closes an interrupt transfer for I2C slave.
 *
 * @details
 * Disables buffer interrupts and cleans minimum necessary interrupt state
 * in handle. Event and error interrupts are left enabled so the slave
 * can continue responding to address matches.
 *
 * @param[in] pI2CHandle    Pointer to handle for I2C peripheral.
 *
 * @return	None.
 ***************************************************************************/
static void I2C_CloseTransferSlave(I2C_handle_t *pI2CHandle)
{
    if (!pI2CHandle || !pI2CHandle->pI2Cx) return;

    // disable buffer interrupts
    // leave event/error interrupts enabled for future addr matches
    pI2CHandle->pI2Cx->CR2 &= ~(1U << I2C_CR2_ITBUFEN_BIT);
            
    // clean handle state
    pI2CHandle->TxRxState       = I2C_READY;
    pI2CHandle->targetAddr      = 0x00U;
}

/**************************************************************************
 * @fn		I2C_EnterRecoveryRequired
 *
 * @brief	Puts interrupt driven I2C handle into recovery-required state.
 *
 * @details
 * Disables all interrupts, sets state to I2C_RECOVERY_REQUIRED, and
 * leaves recovery responsiblity to user's callback.
 *
 * @param[in] pI2CHandle    Pointer to handle for I2C peripheral.
 *
 * @return	None.
 ***************************************************************************/
static void I2C_EnterRecoveryRequired(I2C_handle_t *pI2CHandle)
{
    // disable interrupts
    I2C_InterruptControl(pI2CHandle->pI2Cx, DISABLE);
    // set handle state
    pI2CHandle->TxRxState = I2C_RECOVERY_REQUIRED;
    // notify user
    I2C_ApplicationEventCallback(pI2CHandle, I2C_EVT_INVALID_STATE);
}

/**************************************************************************
 * @fn		I2C_HandleSBMaster
 *
 * @brief	Handles SB interrupts be executing the address phase.
 *
 * @param[in] pI2CHandle    Pointer to handle for I2C peripheral.
 *
 * @return	None.
 *
 * @note	SB flag can only be set if the peripheral is in master mode.
 ***************************************************************************/
static void I2C_HandleSBMaster(I2C_handle_t *pI2CHandle)
{
    // execute address phase
    I2C_rw_t rw;
    if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
    {
        rw = I2C_READ;
    }
    else if(pI2CHandle->TxRxState == I2C_BUSY_IN_TX)
    {
        rw = I2C_WRITE;
    }
    // invalid state
    else
    {
        I2C_EnterRecoveryRequired(pI2CHandle);
        return; 
    }

    I2C_ExecuteAddrPhase(pI2CHandle->pI2Cx, pI2CHandle->targetAddr, rw);
}

/**************************************************************************
 * @fn		I2C_HandleADDRMaster
 *
 * @brief	Handles ADDR interrupts for master mode.
 *
 * @details
 * In the case of a master-in-tx, the address flag is simply cleared and
 * the handler returns.
 *
 * In the case of a master-in-rx, the correct protocol is dependant
 * on the original message size.
 *
 * @param[in] pI2CHandle    Pointer to handle for I2C peripheral.
 *
 * @return	None.
 ***************************************************************************/
static void I2C_HandleADDRMaster(I2C_handle_t *pI2CHandle)
{
    // invalid state
    if (pI2CHandle->TxRxState != I2C_BUSY_IN_TX &&
        pI2CHandle->TxRxState != I2C_BUSY_IN_RX)
    {
        I2C_EnterRecoveryRequired(pI2CHandle);
        return;
    }   

    // if master-in-Tx
    if ( pI2CHandle->TxRxState == I2C_BUSY_IN_TX )
    {
        I2C_ClearADDRFlag(pI2CHandle->pI2Cx);
        return;
    }
    
    // if master-in-Rx
    else if ( pI2CHandle->TxRxState == I2C_BUSY_IN_RX )
    {
        if(pI2CHandle->transferSize == 1U)
        {
            // Disable ACKing
            I2C_ACKControl(pI2CHandle->pI2Cx, DISABLE);

            // Clear ADDR flag
            I2C_ClearADDRFlag(pI2CHandle->pI2Cx);

            // Request stop
            if (pI2CHandle->Sr == I2C_SR_DI)
                I2C_RequestStopCondition(pI2CHandle->pI2Cx);
        }
        else if (pI2CHandle->transferSize == 2U)
        {
            // Assert POS
            pI2CHandle->pI2Cx->CR1 |= (1U << I2C_CR1_POS_BIT);

            // Disable ACKing
            I2C_ACKControl(pI2CHandle->pI2Cx, DISABLE);

            // Clear ADDR flag
            I2C_ClearADDRFlag(pI2CHandle->pI2Cx);
        }
        else // transferSize > 2U
        {
            I2C_ClearADDRFlag(pI2CHandle->pI2Cx);
        }
    }
}

/**************************************************************************
 * @fn		I2C_HandleADDRSlave
 *
 * @brief	Handles ADDR interrupts for slave.
 *
 * @details
 * In the case of a slave, this is the handler which uses the address
 * information from the master to determine whether its going to 
 * receive or transmit. Therefore, the callback must update the
 * necessary handler state to reflect this.
 *
 * @param[in] pI2CHandle    Pointer to handle for I2C peripheral.
 *
 * @return	None.
 ***************************************************************************/
static void I2C_HandleADDRSlave(I2C_handle_t *pI2CHandle)
{
    // invalid state
    if (pI2CHandle->TxRxState != I2C_BUSY_IN_TX &&
        pI2CHandle->TxRxState != I2C_BUSY_IN_RX &&
        pI2CHandle->TxRxState != I2C_READY)
    {
        I2C_EnterRecoveryRequired(pI2CHandle);
        return;
    }   
    
    // disable buffer interrupts
    pI2CHandle->pI2Cx->CR2 &= ~(1U << I2C_CR2_ITBUFEN_BIT);

    // drain pending DR
    if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
    {
        while(pI2CHandle->pI2Cx->SR1 & (1U << I2C_SR1_RxNE_BIT))
            I2C_HandleRxNESlave(pI2CHandle);
    }

        I2C_ClearADDRFlag(pI2CHandle->pI2Cx);

        uint32_t TRA = (pI2CHandle->pI2Cx->SR2 >> I2C_SR2_TRA_BIT) &0x1U;
    uint8_t state = TRA ? I2C_BUSY_IN_TX : I2C_BUSY_IN_RX;

    pI2CHandle->TxRxState = state;

        I2C_ApplicationEventCallback(pI2CHandle, I2C_EVT_SLAVE_ADDR_MATCH);

    // ensure valid handle after callback
    if ((pI2CHandle->TxRxState != state) ||
        (pI2CHandle->pBuffer == NULL && pI2CHandle->remaining > 0U))
    {
        I2C_EnterRecoveryRequired(pI2CHandle);
        return;
    }
    
    // re-enable buffer interrupts
    pI2CHandle->pI2Cx->CR2 |= (1U << I2C_CR2_ITBUFEN_BIT);
}

/**************************************************************************
 * @fn		I2C_HandleBTFMaster
 *
 * @brief	Handles BTF interrupts for master.
 *
 * @details
 * For a master-in-transmit, the BTF flag indicates that both the DR
 * and the shift register is empty. If this occurs because the master
 * has already transmitted all data, then BTF being set indicates that
 * the transfer has completed sending the final byte to the slave. This
 * handler handles such a case by requesting the stop condition and
 * notifying the application.
 * 
 * For a master-in-receive, the proper sequence is dependant on the
 * number of bytes left to receive. Single byte transfers are handled
 * by the RxNE handler. Transfers with exactly two bytes are handled
 * by this BTF handler. Once a tranfer has exactly three bytes
 * remaining, it disables ACKing, reads one byte, and handles the
 * final two bytes with this same BTF handler.
 *
 * @param[in] pI2CHandle    Pointer to handle for I2C peripheral.
 *
 * @return	None.
 ***************************************************************************/
static void I2C_HandleBTFMaster(I2C_handle_t *pI2CHandle)
{
    // if master-in-tx
    if ( pI2CHandle->TxRxState == I2C_BUSY_IN_TX )
    {
        if (pI2CHandle->remaining == 0U)
        {
            // request stop
            if (pI2CHandle->Sr == I2C_SR_DI)
                I2C_RequestStopCondition(pI2CHandle->pI2Cx);
            
            // Close transfer and notify user
            I2C_CloseTransferMaster(pI2CHandle);
            I2C_ApplicationEventCallback(pI2CHandle, I2C_EVT_TX_DONE);
            return;
        }
        else
        {
            I2C_HandleTxEMaster(pI2CHandle);
            return;
        }
    }

    // if master-in-rx
    else if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
    {
        // more than 3 bytes
        if (pI2CHandle->remaining > 3U)
        {
            I2C_HandleRxNEMaster(pI2CHandle);
            return;
        }
        // 2-bytes left
        else if (pI2CHandle->remaining == 2U)
        {
            // Request stop
            if (pI2CHandle->Sr == I2C_SR_DI)
            {
                I2C_RequestStopCondition(pI2CHandle->pI2Cx);
            }

            // Read both bytes
            *pI2CHandle->pBuffer = (uint8_t) pI2CHandle->pI2Cx->DR;
            pI2CHandle->remaining--;
            pI2CHandle->pBuffer++;
            *pI2CHandle->pBuffer = (uint8_t) pI2CHandle->pI2Cx->DR;
            pI2CHandle->remaining--;
            pI2CHandle->pBuffer++;

            // Close transfer and notify user
            I2C_CloseTransferMaster(pI2CHandle);
            I2C_ApplicationEventCallback(pI2CHandle, I2C_EVT_RX_DONE);
            return;
        }
        // 3-bytes remaining
        else if (pI2CHandle->remaining == 3U)
        {
            // Disable ACKing
            I2C_ACKControl(pI2CHandle->pI2Cx, DISABLE);

            // Read data into buffer
            *pI2CHandle->pBuffer = (uint8_t) pI2CHandle->pI2Cx->DR;
            pI2CHandle->pBuffer++;
            pI2CHandle->remaining--;
            return;
        }
    }
    // invalid state
    I2C_EnterRecoveryRequired(pI2CHandle);
}

/**************************************************************************
 * @fn		I2C_HandleSTOPF
 *
 * @brief	TODO
 *
 * @details
 * TODO
 *
 * @param[in] TODO		TODO
 *
 * @return	None.
 *
 * @note	TODO
 ***************************************************************************/
static void I2C_HandleSTOPF(I2C_handle_t *pI2CHandle)
{
    // Clear STOPF flag
    (void) pI2CHandle->pI2Cx->SR1;
    pI2CHandle->pI2Cx->CR1 |= (1U << I2C_CR1_PE_BIT);

    // TODO: Close transfer and notify user of STOP
    I2C_ApplicationEventCallback(pI2CHandle, I2C_EVT_STOP);
}

/**************************************************************************
 * @fn		I2C_HandleTxE
 *
 * @brief	TODO
 *
 * @details
 * TODO
 *
 * @param[in] TODO		TODO
 *
 * @return	None.
 *
 * @note	TODO
 ***************************************************************************/
static void I2C_HandleTxE(I2C_handle_t *pI2CHandle)
{
    if (pI2CHandle->TxRxState == I2C_BUSY_IN_TX)
    {
        if (pI2CHandle->TxLen > 0U)
        {
            pI2CHandle->pI2Cx->DR = *pI2CHandle->pTxBuffer;
            pI2CHandle->pTxBuffer++;
            pI2CHandle->TxLen--;
        }

        if (pI2CHandle->TxLen == 0U)
        {
            // disable buffer interrupts so BTF can notify user if in master mode
            // if in slave-mode, transaction is completed in AF/STOPF handlers
            pI2CHandle->pI2Cx->CR2 &= ~(1U << I2C_CR2_ITBUFEN_BIT);
        }
    }
}

/**************************************************************************
 * @fn		I2C_HandleRxNE
 *
 * @brief	TODO
 *
 * @details
 * TODO
 *
 * @param[in] TODO		TODO
 *
 * @return	None.
 *
 * @note	TODO
 ***************************************************************************/
static void I2C_HandleRxNE(I2C_handle_t *pI2CHandle)
{
    // if master-in-rx
    if (pI2CHandle->mode == I2C_MODE_MASTER &&
        pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
    {
        // one-byte transfer
        if (pI2CHandle->RxSize == 1U)
        {
            *pI2CHandle->pRxBuffer = (uint8_t) pI2CHandle->pI2Cx->DR;
            pI2CHandle->pRxBuffer++;
            pI2CHandle->RxLen--;

            // TODO: Close transfer and notify user
            I2C_ApplicationEventCallback(pI2CHandle, I2C_EVT_RX_DONE);
            return;
        }
        else if (pI2CHandle->RxLen > 3U)
        {
            *pI2CHandle->pRxBuffer = (uint8_t) pI2CHandle->pI2Cx->DR;
            pI2CHandle->pRxBuffer++;
            pI2CHandle->RxLen--;
        }

        if (pI2CHandle->RxLen == 3U)
        {
            // disable buffer interrupts to handle final 3 bytes with BTF handler
            pI2CHandle->pI2Cx->CR2 &= ~(1U << I2C_CR2_ITBUFEN_BIT);
        }
    }
    // if slave-in-rx
    else if (pI2CHandle->mode == I2C_MODE_SLAVE &&
        pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
    {
        if(pI2CHandle->RxLen > 0U)
        {
            *pI2CHandle->pRxBuffer = (uint8_t) pI2CHandle->pI2Cx->DR;
            pI2CHandle->pRxBuffer++;
            pI2CHandle->RxLen--;
        }
        else
        {
            // TODO: record buffer overflow
            (void)pI2CHandle->pI2Cx->DR;
            return;
        }
    }
}

// ***************************************************************
// ******************* INTERRUPT ERROR HELPERS *******************
// ***************************************************************

static void I2C_HandleBERR(I2C_handle_t *pI2CHandle)
{
    // clear BERR
    pI2CHandle->pI2Cx->SR1 &= ~(1U << I2C_SR1_BERR_BIT);

    // disable further interrupts
    pI2CHandle->pI2Cx->CR2 &= ~(1U << I2C_CR2_ITBUFEN_BIT);
    pI2CHandle->pI2Cx->CR2 &= ~(1U << I2C_CR2_ITEVTEN_BIT);
    pI2CHandle->pI2Cx->CR2 &= ~(1U << I2C_CR2_ITERREN_BIT);

    // TODO: Notify application
    I2C_ApplicationEventCallback(pI2CHandle, I2C_EVT_BERR);
}

static void I2C_HandleARLO(I2C_handle_t *pI2CHandle)
{
    // clear ARLO
    pI2CHandle->pI2Cx->SR1 &= ~(1U << I2C_SR1_ARLO_BIT);

    pI2CHandle->mode = I2C_MODE_NONE;

    // disable further interrupts
    pI2CHandle->pI2Cx->CR2 &= ~(1U << I2C_CR2_ITBUFEN_BIT);
    pI2CHandle->pI2Cx->CR2 &= ~(1U << I2C_CR2_ITEVTEN_BIT);
    pI2CHandle->pI2Cx->CR2 &= ~(1U << I2C_CR2_ITERREN_BIT);
    
    // TODO: Notify application
    I2C_ApplicationEventCallback(pI2CHandle, I2C_EVT_ARLO);
}

static void I2C_HandleAF(I2C_handle_t *pI2CHandle)
{
    // if master
    if (pI2CHandle->mode == I2C_MODE_MASTER)
    {
        // request stop
        if (I2C_GetFlag(pI2CHandle->pI2Cx, I2C_MSL_FLAG))
            I2C_RequestStopCondition(pI2CHandle->pI2Cx);

        // clear AF flag
        pI2CHandle->pI2Cx->SR1 &= ~(1U << I2C_SR1_AF_BIT);

        // TODO: close transfer and notify user
        I2C_ApplicationEventCallback(pI2CHandle, I2C_EVT_NACK);
    }
    
    // if slave-in-tx
    else if(pI2CHandle->mode == I2C_MODE_SLAVE &&
            pI2CHandle->TxRxState == I2C_BUSY_IN_TX)
    {
        // clear AF flag
        pI2CHandle->pI2Cx->SR1 &= ~(1U << I2C_SR1_AF_BIT);

        // TODO: Close transfer and notify completion
        I2C_ApplicationEventCallback(pI2CHandle, I2C_EVT_TX_DONE);
    }

    // force clear AF
    else
    {
        pI2CHandle->pI2Cx->SR1 &= ~(1U << I2C_SR1_AF_BIT);
    }
}

static void I2C_HandleOVR(I2C_handle_t *pI2CHandle)
{

}

// static void I2C_HandlePECERR(I2C_handle_t *pI2CHandle)
// {

// }

static void I2C_HandleTIMEOUT(I2C_handle_t *pI2CHandle)
{
    if (pI2CHandle->mode == I2C_MODE_MASTER)
    {

    }
}

// static void I2C_HandleSMBALERT(I2C_handle_t *pI2CHandle)
// {

// }

// ***************************************************************
// **************** INTERRUPT API IMPLEMENTATIONS ****************
// ***************************************************************

/**************************************************************************
 * @fn		I2C_MasterSendDataIT
 *
 * @brief	Saves interrupt state for Tx, requests start, and enables
 *          interrupts.
 *
 * @param[in] pI2CHandle    Handle for a I2C peripheral.
 * @param[in] pTxBuffer		Pointer to data-buffer to be transfered.
 * @param[in] len		    Number of bytes to transfer.
 * @param[in] slaveAddr		Target slave address to transfer to.
 * @param[in] SR		    Boolean indicating whether to send stop-
 *                          condition (I2C_SR_DI) or not (I2C_SR_EN).
 *
 * @return	STATUS_OK: Interrupt state was successfully saved in 
 *          handle,start condition was requested and interrupts enabled.
 *			STATUS_INVALID_PARAM: An invalid argument was passed.
 *          STATUS_BUSY: The passed I2C peripheral is busy.
 *
 * @note	This API does not transmit any actual data. It only saves
 *          the necessary state in the handle, enables interrupts, and
 *          requests a start condition.
 *
 * @note    This API enables all three interrupt flags in the CR2
 *          register (ITBUFEN, ITEVTEN, ITERREN).
 ***************************************************************************/
status_t I2C_MasterSendDataIT(I2C_handle_t *pI2CHandle,               
    uint8_t *pTxBuffer, uint32_t len, uint8_t slaveAddr,
    uint8_t SR)
{
    // Validate pointers
    if (!pI2CHandle || !pI2CHandle->pI2Cx ||
        !I2C_IsValidInstance(pI2CHandle->pI2Cx))
        return STATUS_INVALID_PARAM;

    // ensure handle is free
    if (pI2CHandle->TxRxState != I2C_READY)
        return STATUS_BUSY;

    // validate inputs
    if (!pTxBuffer || len < 1U)
        return STATUS_INVALID_PARAM;

    if (slaveAddr > 0x7FU || SR > 1U)
        return STATUS_INVALID_PARAM;

    // Update interrupt state in handle
    pI2CHandle->pTxBuffer   = pTxBuffer;
    pI2CHandle->TxLen       = len;
    pI2CHandle->TxRxState   = I2C_BUSY_IN_TX;
    pI2CHandle->targetAddr  = slaveAddr;
    pI2CHandle->Sr          = SR;
    pI2CHandle->mode        = I2C_MODE_MASTER;

    // Request start condition
    I2C_RequestStartCondition(pI2CHandle->pI2Cx);

    // Enable interrupts
    pI2CHandle->pI2Cx->CR2 |= 
        (1U << I2C_CR2_ITEVTEN_BIT) |
        (1U << I2C_CR2_ITERREN_BIT) |
        (1U << I2C_CR2_ITBUFEN_BIT);

    return STATUS_OK;
}

/**************************************************************************
 * @fn		I2C_MasterReceiveDataIT
 *
 * @brief	Saves interrupt state for Rx, requests start, and enables
 *          interrupts.
 *
 * @param[in] pI2CHandle    Pointer to handle for an I2C peripheral.
 * @param[in] pRxBuffer		Pointer to Rx buffer.
 * @param[in] len		    Length of bytes to receive.
 * @param[in] slaveAddr		Address of slave to receive data from.
 * @param[in] SR		    Boolean indicating whether to send stop-
 *                          condition (I2C_SR_DI) or not (I2C_SR_EN).
 *
 * @return	STATUS_OK: Interrupt state was successfully saved in 
 *          handle, start condition was requested, and interrupts enabled.
 *			STATUS_INVALID_PARAM: An invalid argument was passed.
 *          STATUS_BUSY: The passed I2C peripheral is busy.
 *
 * @note	This API does not receive any actual data. It only saves
 *          the necessary state in the handle, enables interrupts, and
 *          requests a start condition.
 *
 * @note    This API enables all three interrupt flags in the CR2
 *          register (ITBUFEN, ITEVTEN, ITERREN).
 ***************************************************************************/
status_t I2C_MasterReceiveDataIT(I2C_handle_t *pI2CHandle,
    uint8_t *pRxBuffer, uint32_t len, uint8_t slaveAddr,
    uint8_t SR)
{
    // Validate pointers
    if (!pI2CHandle || !pI2CHandle->pI2Cx ||
        !I2C_IsValidInstance(pI2CHandle->pI2Cx))
        return STATUS_INVALID_PARAM;

    // ensure handle is free
    if (pI2CHandle->TxRxState != I2C_READY)
        return STATUS_BUSY;

    // validate inputs
    if (!pRxBuffer || len < 1U)
        return STATUS_INVALID_PARAM;

    if (slaveAddr > 0x7FU || SR > 1U)
        return STATUS_INVALID_PARAM;

    // Update interrupt state in handle
    pI2CHandle->pRxBuffer   = pRxBuffer;
    pI2CHandle->RxLen       = len;
    pI2CHandle->TxRxState   = I2C_BUSY_IN_RX;
    pI2CHandle->targetAddr  = slaveAddr;
    pI2CHandle->RxSize      = len;
    pI2CHandle->Sr          = SR;
    pI2CHandle->mode        = I2C_MODE_MASTER;

    // Enable ACKing
    pI2CHandle->pI2Cx->CR1 |= (1U << I2C_CR1_ACK_BIT);

    // Clear POS
    pI2CHandle->pI2Cx->CR1 &= ~(1U << I2C_CR1_POS_BIT);

    // Request start condition
    I2C_RequestStartCondition(pI2CHandle->pI2Cx);

    // conditional bit-mask. 1 byte uses RxNE, two/three bytes wait for BTF
    uint32_t mask = (1U << I2C_CR2_ITEVTEN_BIT) | (1U << I2C_CR2_ITERREN_BIT);
    if (len == 1U || len > 3U)
    {
        mask |= (1U << I2C_CR2_ITBUFEN_BIT);
    }

    // Enable interrupts
    // first unconditionally clears ITBUFEN, and then sets it according to its value in mask
    pI2CHandle->pI2Cx->CR2 = (pI2CHandle->pI2Cx->CR2 & ~(1U << I2C_CR2_ITBUFEN_BIT)) | mask;

    return STATUS_OK;
}

/**************************************************************************
 * @fn		I2C_IRQHandling_EV
 *
 * @brief	TODO
 *
 * @details
 * TODO
 *
 * @param[in] TODO		TODO
 *
 * @return	None.
 *
 * @note	This ISR is used for both master and slave mode I2C. 
 ***************************************************************************/
status_t I2C_IRQHandling_EV(I2C_handle_t *pI2CHandle)
{
    // Validate pointers
    if (!pI2CHandle || !pI2CHandle->pI2Cx ||
        !I2C_IsValidInstance(pI2CHandle->pI2Cx))
        return STATUS_INVALID_PARAM;

    // cache registers
    uint32_t SR1 = pI2CHandle->pI2Cx->SR1;
    uint32_t CR2 = pI2CHandle->pI2Cx->CR2;

    // handle SB interrupt (MASTER-ONLY)
    if (CR2 & (1U << I2C_CR2_ITEVTEN_BIT) &&
        SR1 & (1U << I2C_SR1_SB_BIT))
    {
        I2C_HandleSB(pI2CHandle);
    }

    // update cached registers
    SR1 = pI2CHandle->pI2Cx->SR1;
    CR2 = pI2CHandle->pI2Cx->CR2;

    // Handle ADDR interrupt
    // Master:  Address successfully sent
    // Slave:   Address on bus matches own address
    if (CR2 & (1U << I2C_CR2_ITEVTEN_BIT) &&
        SR1 & (1U << I2C_SR1_ADDR_BIT))
    {
        I2C_HandleADDR(pI2CHandle);
    }

    // Update handle mode field
    pI2CHandle->mode =  I2C_GetFlag(pI2CHandle->pI2Cx, I2C_MSL_FLAG) ?
                        I2C_MODE_MASTER : I2C_MODE_SLAVE;

    // update cached registers
    SR1 = pI2CHandle->pI2Cx->SR1;
    CR2 = pI2CHandle->pI2Cx->CR2;

    // Handle BTF interrupt
    if (CR2 & (1U << I2C_CR2_ITEVTEN_BIT) &&
        SR1 & (1U << I2C_SR1_BTF_BIT))
    {
        I2C_HandleBTF(pI2CHandle);
    }

    // update cached registers
    SR1 = pI2CHandle->pI2Cx->SR1;
    CR2 = pI2CHandle->pI2Cx->CR2;

    // handle TxE interrupt
    if (CR2 & (1U << I2C_CR2_ITEVTEN_BIT) &&
        CR2 & (1U << I2C_CR2_ITBUFEN_BIT) &&
        SR1 & (1U << I2C_SR1_TxE_BIT))
    {
        I2C_HandleTxE(pI2CHandle);
    }

    // update cached registers
    SR1 = pI2CHandle->pI2Cx->SR1;
    CR2 = pI2CHandle->pI2Cx->CR2;

    // Handle RxNE interrupt
    if (CR2 & (1U << I2C_CR2_ITEVTEN_BIT) &&
        CR2 & (1U << I2C_CR2_ITBUFEN_BIT) &&
        SR1 & (1U << I2C_SR1_RxNE_BIT))
    {
        I2C_HandleRxNE(pI2CHandle);
    }

    // update cached registers
    SR1 = pI2CHandle->pI2Cx->SR1;
    CR2 = pI2CHandle->pI2Cx->CR2;

    // Handle STOPF interrupt (SLAVE-ONLY)
    // set when slave detects a stop after an ACK
    // this is handled after RxNE to not lose the final byte
    if (CR2 & (1U << I2C_CR2_ITEVTEN_BIT) &&
        SR1 & (1U << I2C_SR1_STOPF_BIT))
    {
        I2C_HandleSTOPF(pI2CHandle);
    }
    
    return STATUS_OK;
}

/**************************************************************************
 * @fn		I2C_IRQHandling_ERR
 *
 * @brief	TODO
 *
 * @details
 * TODO
 *
 * @param[in] TODO		TODO
 *
 * @return	None.
 *
 * @note	TODO
 ***************************************************************************/
status_t I2C_IRQHandling_ERR(I2C_handle_t *pI2CHandle)
{
    // Validate pointers
    if (!pI2CHandle || !pI2CHandle->pI2Cx ||
        !I2C_IsValidInstance(pI2CHandle->pI2Cx))
        return STATUS_INVALID_PARAM;

    // cache registers
    uint32_t SR1 = pI2CHandle->pI2Cx->SR1;
    uint32_t CR2 = pI2CHandle->pI2Cx->CR2;

    // handle bus error
    if (CR2 & (1U << I2C_CR2_ITERREN_BIT) &&
        SR1 & (1U << I2C_SR1_BERR_BIT))
    {
        I2C_HandleBERR(pI2CHandle);
        return STATUS_ERROR;
    }

    // handle arbitration error
    if (CR2 & (1U << I2C_CR2_ITERREN_BIT) &&
        SR1 & (1U << I2C_SR1_ARLO_BIT))
    {
        I2C_HandleARLO(pI2CHandle);
    }

    // handle NACK error
    if (CR2 & (1U << I2C_CR2_ITERREN_BIT) &&
        SR1 & (1U << I2C_SR1_AF_BIT))
    {
        I2C_HandleAF(pI2CHandle);
    }

    // handle over/under-run error
    if (CR2 & (1U << I2C_CR2_ITERREN_BIT) &&
        SR1 & (1U << I2C_SR1_OVR_BIT))
    {
        I2C_HandleOVR(pI2CHandle);
    }

    // handle timeout error
    if (CR2 & (1U << I2C_CR2_ITERREN_BIT) &&
        SR1 & (1U << I2C_SR1_TIMEOUT_BIT))
    {
        I2C_HandleTIMEOUT(pI2CHandle);
    }

    return STATUS_OK;
}
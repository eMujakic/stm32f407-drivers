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
 * @fn		I2C_WaitForFlagErr
 *
 * @brief	Spins until flag==val, software timeout is triggered, or an
 *          error flag (ARLO, BERR, AF, OVR, PECERR) is asserted.
 *
 * @param[in] pI2Cx		Pointer to I2C's base address.
 * @param[in] flag		Single-bit flag to poll.
 * @param[in] val		Desired value for flag.
 * @param[in] errFlag   Pointer to store error flag.
 *
 * @return	STATUS_OK: flag == val before software timeout.
 *          STATUS_TIMEOUT: Software timeout triggered.
 *			STATUS_INVALID_PARAM: Invalid input.
 *
 * @note	Does not handle waiting for a specific value on 
 *          any SR2 flags to avoid accidental clears of ADDR.
 *
 * @note    Error flags are checked according to the order in 
 *          the brief. i.e. (ARLO, BERR, AF, OVR, PECERR).
 ***************************************************************************/
static status_t I2C_WaitForFlagErr(I2C_reg_t *pI2Cx, I2C_flag_t flag, uint8_t val, I2C_flag_t *errFlag)
{
    // validate args
    if (!pI2Cx || !I2C_IsValidInstance(pI2Cx) ||
        flag < I2C_SB_FLAG || flag > I2C_SMBALERT_FLAG ||
        val > 1U)
    {
        return STATUS_INVALID_PARAM;
    }

    if (!errFlag)
        return STATUS_INVALID_PARAM;
    *errFlag = I2C_NOERR_FLAG;
        
    uint32_t timeout = I2C_TIMEOUT;
    while(I2C_GetFlag(pI2Cx, flag) != val)
    {
        if (I2C_GetFlag(pI2Cx, I2C_ARLO_FLAG))
        {
            *errFlag = I2C_ARLO_FLAG;
            return STATUS_ERROR;
        }
        if (I2C_GetFlag(pI2Cx, I2C_BERR_FLAG))
        {
            *errFlag = I2C_BERR_FLAG;
            return STATUS_ERROR;
        }
        if (I2C_GetFlag(pI2Cx, I2C_AF_FLAG))
        {
            *errFlag = I2C_AF_FLAG;
            return STATUS_ERROR;
        }
        if (I2C_GetFlag(pI2Cx, I2C_OVR_FLAG))
        {
            *errFlag = I2C_OVR_FLAG;
            return STATUS_ERROR;
        }
        if (I2C_GetFlag(pI2Cx, I2C_PECERR_FLAG))
        {
            *errFlag = I2C_PECERR_FLAG;
            return STATUS_ERROR;
        }

        if (timeout == 0U) return STATUS_TIMEOUT;
        timeout--;
    }

    return STATUS_OK;
}

static status_t I2C_MasterErrHandling(I2C_reg_t *pI2Cx, status_t status, I2C_flag_t errFlag)
{
    // validate inputs
    if (!pI2Cx || !I2C_IsValidInstance(pI2Cx))
        return STATUS_INVALID_PARAM;

    if (status != STATUS_ERROR &&
        status != STATUS_TIMEOUT)
        return STATUS_INVALID_PARAM;

    if (status == STATUS_ERROR)
    {
        if (errFlag == I2C_ARLO_FLAG) // arbitration error
        {
            pI2Cx->SR1 &= ~(1U << I2C_SR1_ARLO_BIT); // clear ARLO flag
            return status;
        }
        if (errFlag == I2C_BERR_FLAG) // bus error
        {
            pI2Cx->SR1 &= ~(1U << I2C_SR1_BERR_BIT); // clear BERR flag
        }
        if (errFlag == I2C_PECERR_FLAG) // PEC mismatch
        {
            pI2Cx->SR1 &= ~(1U << I2C_SR1_PECERR_BIT); // clear PECERR flag
        }
    }
    if (I2C_GetFlag(pI2Cx, I2C_MSL_FLAG)) // request stop condition if still master
        I2C_RequestStopCondition(pI2Cx);

    if (status == STATUS_ERROR && errFlag == I2C_AF_FLAG)
        pI2Cx->SR1 &= ~(1U << I2C_SR1_AF_BIT); // clear AF flag AFTER requesting stop

    return status;
}

// ***************************************************************
// ********************* API IMPLEMENTATIONS *********************
// ***************************************************************

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
 * @brief	Spins until flag==val or software timeout is triggered.
 *
 * @param[in] pI2Cx		Pointer to I2C's base address.
 * @param[in] flag		Single-bit flag to poll.
 * @param[in] val		Desired value for flag.
 *
 * @return	STATUS_OK: flag == val before software timeout.
 *          STATUS_BUSY: Software timeout triggered.
 *			STATUS_INVALID_PARAM: Invalid input.
 *
 * @note	Does not handle waiting for a specific value on 
 *          I2C_PEC_FLAG.
 ***************************************************************************/
status_t I2C_WaitForFlag(I2C_reg_t *pI2Cx, I2C_flag_t flag, uint8_t val)
{
    // validate args
    if (!pI2Cx || !I2C_IsValidInstance(pI2Cx) ||
        flag < I2C_SB_FLAG || flag >= I2C_PEC_FLAG ||
        val > 1U)
    {
        return STATUS_INVALID_PARAM;
    }
        
    uint32_t timeout = I2C_TIMEOUT;
    while(I2C_GetFlag(pI2Cx, flag) != val)
    {
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
    else                    return STATUS_INVALID_PARAM;
        
    return STATUS_OK;
}

/**************************************************************************
 * @fn		I2C_MasterSendData
 *
 * @brief	Sends data as master using I2C protocol.
 *
 * @details
 * TODO
 *
 * @param[in] pI2CHandle    TODO
 * @param[in] pTxBuffer		Pointer to data-buffer to be transfered.
 * @param[in] len		    Number of bytes to transfer.
 * @param[in] slaveAddr		Target slave address to transfer to.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_ERROR / STATUS_INVALID_PARAM).
 *
 * @note	TODO
 ***************************************************************************/
status_t I2C_MasterSendData(I2C_handle_t *pI2CHandle, uint8_t *pTxBuffer,
                            uint32_t len, uint8_t slaveAddr)
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
        return status;

    // ========= Send Address and R/W Bit =========
    I2C_ExecuteAddrPhase(pI2CHandle->pI2Cx, slaveAddr, I2C_WRITE); // writes DR

    // ========= Confirm Address Phase Completion =========
    I2C_flag_t errFlag = I2C_NOERR_FLAG;
    status = I2C_WaitForFlagErr(pI2CHandle->pI2Cx, I2C_ADDR_FLAG, SET, &errFlag);
    if (status != STATUS_OK)
    {
        return I2C_MasterErrHandling(pI2CHandle->pI2Cx, status, errFlag);
    }

    // ========= Clear ADDR =========
    I2C_ClearADDRFlag(pI2CHandle->pI2Cx);

    // ========= Send Data =========
    while(len > 0)
    {
        errFlag = I2C_NOERR_FLAG;
        status = I2C_WaitForFlagErr(pI2CHandle->pI2Cx, I2C_TxE_FLAG, SET, &errFlag); // wait until Tx buffer is empty
        if (status != STATUS_OK)
            {
                return I2C_MasterErrHandling(pI2CHandle->pI2Cx, status, errFlag);
            }
        
        pI2CHandle->pI2Cx->DR = *pTxBuffer;
        pTxBuffer++;
        len--;
    }

    // ========= Close Transmission =========
    errFlag = I2C_NOERR_FLAG;
    status = I2C_WaitForFlagErr(pI2CHandle->pI2Cx, I2C_BTF_FLAG, SET, &errFlag); // wait until byte-transfer complete
    if (status != STATUS_OK)
    {
        return I2C_MasterErrHandling(pI2CHandle->pI2Cx, status, errFlag);
    }

    // ========= Request Stop Condition =========
    I2C_RequestStopCondition(pI2CHandle->pI2Cx);
    return STATUS_OK;
}
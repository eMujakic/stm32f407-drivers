/*
 * stm32f407xx_usart_driver.c
 *
 *  Created on: Nov 13, 2026
 *      Author: ernad
 */

#include "stm32f407xx_usart_driver.h"

// ***************************************************************
// ******************* PRIVATE HELPER FUNCTIONS ******************
// ***************************************************************

/**************************************************************************
 * @fn		USART_IsValidInstance
 *
 * @brief	Used to determine whether arg is a valid USART base addr.
 *
 * @param[in] pUSARTx   USART peripheral base address.
 *
 * @return	Boolean indicating validity of passed USART base addr.
 ***************************************************************************/
static inline uint8_t USART_IsValidInstance(USART_reg_t *pUSARTx)
{
    return 
    (
        (pUSARTx == USART1) || 
        (pUSARTx == USART2) || 
        (pUSARTx == USART3) ||
        (pUSARTx == UART4)  ||
        (pUSARTx == UART5)  ||
        (pUSARTx == USART6)
    );
}

/**************************************************************************
 * @fn		USART_IsUART
 *
 * @brief	Used to determine whether a base address is UART4/UART5.
 *
 * @param[in] pUSARTx   USART peripheral base address.
 *
 * @return	Boolean indicating whether passed pointer is UART4/UART5.
 ***************************************************************************/
static inline uint8_t USART_IsUART(USART_reg_t *pUSARTx)
{
    return(
        (pUSARTx == UART4)  ||
        (pUSARTx == UART5)
    );
}

// ***************************************************************
// ********************* API IMPLEMENTATIONS *********************
// ***************************************************************

/**************************************************************************
 * @fn		USART_PeriClkCtl
 *
 * @brief	Enables/disables peripheral clock for a USART peripheral.
 *
 * @param[in] pUSARTx   Pointer to a valid USART base address.
 * @param[in] enable    Enable/Disable
 *
 * @return	STATUS_OK: Peripheral clock was successfully
 *          enabled/disabled.
 *          STATUS_INVALID_PARAM: Invalid argument was passed.
 *
 * @note	This does not enable the USART peripheral, it only enables
 *          its clock.
 ***************************************************************************/
status_t USART_PeriClkCtl(USART_reg_t *pUSARTx, uint8_t enable)
{
    // Validate arguments
    if (!USART_IsValidInstance(pUSARTx)) return STATUS_INVALID_PARAM;
    if (enable != ENABLE && enable != DISABLE) return STATUS_INVALID_PARAM;

    if (pUSARTx == USART1)
    {
        if (enable == ENABLE)   USART1_PCLK_EN();
        else                    USART1_PCLK_DI();
    }

    else if (pUSARTx == USART2)
    {
        if (enable == ENABLE)   USART2_PCLK_EN();
        else                    USART2_PCLK_DI();
    }

    else if (pUSARTx == USART3)
    {
        if (enable == ENABLE)   USART3_PCLK_EN();
        else                    USART3_PCLK_DI();
    }

    else if (pUSARTx == UART4)
    {
        if (enable == ENABLE)   UART4_PCLK_EN();
        else                    UART4_PCLK_DI();
    }

    else if (pUSARTx == UART5)
    {
        if (enable == ENABLE)   UART5_PCLK_EN();
        else                    UART5_PCLK_DI();
    }

    else if (pUSARTx == USART6)
    {
        if (enable == ENABLE)   USART6_PCLK_EN();
        else                    USART6_PCLK_DI();
    }

    else return STATUS_INVALID_PARAM;

    return STATUS_OK;
}

/**************************************************************************
 * @fn		USART_PeripheralControl
 *
 * @brief	Enables/disables a USART peripheral.
 *
 * @details
 * Sets the UE (USART-enable) bit in the peripheral's CR1 register.
 *
 * @param[in] pUSARTx   Pointer to a valid USART base address.
 * @param[in] enable    Enable/Disable
 *
 * @return	STATUS_OK: Peripheral was successfully
 *          enabled/disabled.
 *          STATUS_INVALID_PARAM: Invalid argument was passed.
 ***************************************************************************/
status_t USART_PeripheralControl(USART_reg_t *pUSARTx, uint8_t enable)
{
    // validate inputs
    if (!USART_IsValidInstance(pUSARTx))
        return STATUS_INVALID_PARAM;
    if (enable != ENABLE && enable != DISABLE)
        return STATUS_INVALID_PARAM;
    
    if (enable == ENABLE)
    {
        pUSARTx->CR1 |= (1U << USART_CR1_UE_BIT);
    }
    else
    {
        pUSARTx->CR1 &= ~(1U << USART_CR1_UE_BIT);
    }

    return STATUS_OK;
}

/**************************************************************************
 * @fn		USART_ConfigInit
 *
 * @brief	Initializes USART config struct to reasonable default values.
 *
 * @details
 * Values are initialized to the following:
 * USART_Mode: TX & RX
 * USART_Baud: 115200
 * USART_NumStopBits: 1
 * USART_WordLen: 8-bits
 * USART_ParityCtl: disabled
 * USART_HWFlowCtl: none
 * USART_Oversampling: 16
 *
 * @param[in] pConfig   Pointer to USART configuration structure.
 *
 * @return	STATUS_OK: Config values were successfully initialized.
 *          STATUS_INVALID_PARAM: A NULL pointer was passed.
 * 
 * @note    This API only initializes the structure's values. It does not
 *          configure an actual USART peripheral.
 ***************************************************************************/
status_t USART_ConfigInit(USART_config_t *pConfig)
{
    if (!pConfig)
        return STATUS_INVALID_PARAM;

    pConfig->USART_Mode         = USART_MODE_TX_RX;
    pConfig->USART_Baud         = USART_BAUD_115200;
    pConfig->USART_NumStopBits  = USART_STOP_BITS_1;
    pConfig->USART_WordLen      = USART_WORD_LEN_8;
    pConfig->USART_ParityCtl    = USART_PARITY_DISABLED;
    pConfig->USART_HWFlowCtl    = USART_HW_FLOW_CTL_NONE;
    pConfig->USART_Oversampling = USART_OVERSAMPLING_16;

    return STATUS_OK;
}

/**************************************************************************
 * @fn		USART_Reset
 *
 * @brief	Resets a USART peripheral.
 * 
 * @param[in] pUSARTx   Pointer to USART base address.
 *
 * @return	STATUS_OK: Peripheral was successfully reset.
 *          STATUS_INVALID_PARAM: An invalid USART pointer was passed.
 *
 * @note	This does not disable the peripheral clock for the peripheral.
 ***************************************************************************/
status_t USART_Reset(USART_reg_t *pUSARTx)
{
    // validate pointer
    if (!pUSARTx || !USART_IsValidInstance(pUSARTx))
        return STATUS_INVALID_PARAM;

    if (pUSARTx == USART1)
        USART1_RESET();
    else if (pUSARTx == USART2)
        USART2_RESET();
    else if (pUSARTx == USART3)
        USART3_RESET();
    else if (pUSARTx == UART4)
        UART4_RESET();
    else if (pUSARTx == UART5)
        UART5_RESET();
    else if (pUSARTx == USART6)
        USART6_RESET();
    else
        return STATUS_INVALID_PARAM;

    return STATUS_OK;
}

/**************************************************************************
 * @fn		USART_GetFlag
 *
 * @brief	Returns value of USART/UART flag.
 *
 * @param[in] pUSARTx   Pointer to USART base address.
 * @param[in] flag	    Enum value of desired flag to read.
 *
 * @return	The value of flag in the peripheral's status register.
 *
 * @note	Returns 0U if invalid argument is passed.
 ***************************************************************************/
uint8_t USART_GetFlag(USART_reg_t *pUSARTx, USART_flag_t flag)
{
    // validate pointer
    if (!pUSARTx || !USART_IsValidInstance(pUSARTx))
        return 0U;

    // UART4/UART5 does not support CTS flag
    if (USART_IsUART(pUSARTx) &&
        flag == USART_FLAG_CTS)
        return 0U;

    switch(flag)
    {
        case USART_FLAG_PE:
            return ( (pUSARTx->SR >> USART_SR_PE_BIT) & 0x1U );
        case USART_FLAG_FE:
            return ( (pUSARTx->SR >> USART_SR_FE_BIT) & 0x1U );
        case USART_FLAG_NF:
            return ( (pUSARTx->SR >> USART_SR_NF_BIT) & 0x1U );
        case USART_FLAG_ORE:
            return ( (pUSARTx->SR >> USART_SR_ORE_BIT) & 0x1U );
        case USART_FLAG_IDLE:
            return ( (pUSARTx->SR >> USART_SR_IDLE_BIT) & 0x1U );
        case USART_FLAG_RXNE:
            return ( (pUSARTx->SR >> USART_SR_RXNE_BIT) & 0x1U );
        case USART_FLAG_TC:
            return ( (pUSARTx->SR >> USART_SR_TC_BIT) & 0x1U );
        case USART_FLAG_TXE:
            return ( (pUSARTx->SR >> USART_SR_TXE_BIT) & 0x1U );
        case USART_FLAG_LBD:
            return ( (pUSARTx->SR >> USART_SR_LBD_BIT) & 0x1U );
        case USART_FLAG_CTS:
            return ( (pUSARTx->SR >> USART_SR_CTS_BIT) & 0x1U );
        default:
            return 0U;
    }
}

/**************************************************************************
 * @fn		USART_WaitForFlag
 *
 * @brief	Polls until flag == val or polling budget is exceeded.
 *
 * @param[in] pUSARTx	Pointer to USART base address.
 * @param[in] flag		desired flag to poll.
 * @param[in] val		Desired value of flag (0 or 1).
 *
 * @return	STATUS_OK: flag == val
 *          STATUS_INVALID_PARAM: An invalid argument was passed.
 *          STATUS_TIMEOUT: The polling budget was exceeded.
 *
 * @note	This API returns STATUS_INVALID_PARAM if polling for the CTS
 *          flag and the peripheral is a UART peripheral (UART4/UART5).
 ***************************************************************************/
status_t USART_WaitForFlag(USART_reg_t *pUSARTx, USART_flag_t flag, uint8_t val)
{
    // validate arguments
    if (!pUSARTx || !USART_IsValidInstance(pUSARTx))
        return STATUS_INVALID_PARAM;
    
    if (flag < USART_FLAG_PE ||
        flag > USART_FLAG_CTS)
        return STATUS_INVALID_PARAM;

    if (val > 1U)
        return STATUS_INVALID_PARAM;

    if (USART_IsUART(pUSARTx) && flag == USART_FLAG_CTS)
        return STATUS_INVALID_PARAM;
    
    uint32_t timeout = USART_TIMEOUT;
    while(1)
    {
        // check desired flag
        if (USART_GetFlag(pUSARTx, flag) == val)
            break;
            
        // check timeout
        if (timeout == 0U) return STATUS_TIMEOUT;
        timeout--;
    }
    return STATUS_OK;
}


// ***************************************************************
// ********************** INTERRUPT HELPERS **********************
// ***************************************************************


// ***************************************************************
// **************** INTERRUPT API IMPLEMENTATIONS ****************
// ***************************************************************

__weak void USART_ApplicationEventCallback(USART_handle_t *pHandle, USART_event_t event)
{
    (void) pHandle;
    (void) event;
}
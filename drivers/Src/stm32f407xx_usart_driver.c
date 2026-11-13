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

// ***************************************************************
// ********************** INTERRUPT HELPERS **********************
// ***************************************************************


// ***************************************************************
// **************** INTERRUPT API IMPLEMENTATIONS ****************
// ***************************************************************
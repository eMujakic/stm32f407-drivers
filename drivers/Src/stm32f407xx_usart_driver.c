/*
 * stm32f407xx_usart_driver.c
 *
 *  Created on: Nov 13, 2026
 *      Author: ernad
 */

#include "stm32f407xx_usart_driver.h"

// ******************* Function Prototypes ******************
static void USART_DisableInterrupts(USART_reg_t *pUSARTx);

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

/**************************************************************************
 * @fn		USART_WaitForRxNE
 *
 * @brief	Polls for RxNE while checking for errors.
 *
 * @param[in] pUSARTx   USART peripheral base address.
 *
 * @return	STATUS_OK: RxNE was set with no errors detected.
 *          STATUS_ERROR: An error was detected.
 *          STATUS_INVALID_PARAM: An invalid argument was passed.
 *
 * @note	This helper does not clear the error. The caller must void
 *          DR to clear any error flags.
 ***************************************************************************/
static status_t USART_WaitForRxNE(USART_reg_t *pUSARTx)
{
    // validate arguments
    if (!pUSARTx || !USART_IsValidInstance(pUSARTx))
        return STATUS_INVALID_PARAM;
    
    uint32_t error_mask = ( (1U << USART_SR_PE_BIT) |
                            (1U << USART_SR_FE_BIT) |
                            (1U << USART_SR_NF_BIT) |
                            (1U << USART_SR_ORE_BIT));
    
    uint32_t timeout = USART_TIMEOUT;
    while(1)
    {
        uint32_t sr = pUSARTx->SR;

        if (sr & error_mask)
            return STATUS_ERROR;

        // check desired flag
        if (sr & (1U << USART_SR_RXNE_BIT))
            break;
            
        // check timeout
        if (timeout == 0U) return STATUS_TIMEOUT;
        timeout--;
    }
    return STATUS_OK;
}

/**************************************************************************
 * @fn		USART_SetBaudRate
 *
 * @brief	Calculates and sets the mantissa and fraction values in the
 *          BRR register for the desired baud rate.
 *
 * @details
 * Uses PCLK2 for USART1/6, and PCLK1 for all other USART/UART
 * peripherals.
 *
 * @param[in] pUSARTHandle  Pointer to handle for USART/UART peripheral.
 *
 * @return	STATUS_OK: BRR register was successfully configured.
 *          STATUS_INVALID_PARAM: The calculated mantissa is outside the
 *          valid range 1-4095.
 *
 * @note	BRR is left unchanged if STATUS_INVALID_PARAM is returned.
 ***************************************************************************/
static status_t USART_SetBaudRate(USART_handle_t *pUSARTHandle)
{
    USART_reg_t *pUSARTx    = pUSARTHandle->pUSARTx;
    USART_config_t config   = pUSARTHandle->USART_Config;

    // get peripheral clock value
    uint32_t pclk;
    if (pUSARTx == USART1 || pUSARTx == USART6)
        pclk = RCC_GetPCLK2Speed(); // APB2 clock
    else
        pclk = RCC_GetPCLK1Speed(); // APB1 clock

    uint32_t scaled_div = (uint32_t)(((uint64_t)pclk + (config.USART_Baud / 2U)) / (config.USART_Baud));

    uint32_t oversampling   = (16U - (8U * config.USART_Oversampling));
    uint32_t mantissa       = scaled_div / oversampling;
    uint32_t frac           = scaled_div - (mantissa * oversampling);

    if (mantissa > 0xFFFU || mantissa == 0U)
        return STATUS_INVALID_PARAM;
    
    // write value to BRR register
    pUSARTx->BRR = (mantissa << 4U) | frac;

    return STATUS_OK;
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
 * 
 * @note    This API is not error-aware. That is, it does not check error
 *          flags while polling. A STATUS_TIMEOUT or STATUS_OK value may 
 *          be returned even though an error flag is asserted in SR.
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

/**************************************************************************
 * @fn		USART_Init
 *
 * @brief	Initializes USART peripheral according to config structure.
 *
 * @details
 * Validates the handle and configurations, enables peripheral clock,
 * disables interrupt sources, and disables the peripheral before
 * configuring. Configures transmit/receive mode, stop-bits, word length,
 * parity, hardware flow control, oversampling, and baud rate.
 * Initializes runtime handle state and enables the peripheral before 
 * returning.
 *
 * @param[in] pUSARTHandle  Pointer to a USART handle.
 *
 * @return	STATUS_OK: Peripheral was successfully configured.
 *          STATUS_INVALID_PARAM: An invalid argument was passed.
 *
 * @note	This API returns STATUS_INVALID_PARAM if using fraction stop-
 *          bits or hardware flow control on a UART (UART4/5) peripheral.
 * 
 * @note    If baud-rate configuration fails, the peripheral is reset and
 *          STATUS_INVALID_PARAM is returned.
 ***************************************************************************/
status_t USART_Init(USART_handle_t *pUSARTHandle)
{
    // validate inputs
    if (!pUSARTHandle || !pUSARTHandle->pUSARTx)
        return STATUS_INVALID_PARAM;

    if (!USART_IsValidInstance(pUSARTHandle->pUSARTx))
        return STATUS_INVALID_PARAM;

    // validate configuration values
    USART_config_t config = pUSARTHandle->USART_Config;

    if (config.USART_Mode < USART_MODE_TX_ONLY ||
        config.USART_Mode > USART_MODE_TX_RX)
        return STATUS_INVALID_PARAM;

    if (config.USART_Baud == 0U)
        return STATUS_INVALID_PARAM;

    if (config.USART_NumStopBits < USART_STOP_BITS_1 ||
        config.USART_NumStopBits > USART_STOP_BITS_1_5)
        return STATUS_INVALID_PARAM;

    if (config.USART_WordLen < USART_WORD_LEN_8 ||
        config.USART_WordLen > USART_WORD_LEN_9)
        return STATUS_INVALID_PARAM;

    if (config.USART_ParityCtl < USART_PARITY_DISABLED ||
        config.USART_ParityCtl > USART_PARITY_ODD)
        return STATUS_INVALID_PARAM;
    
    if (config.USART_HWFlowCtl < USART_HW_FLOW_CTL_NONE ||
        config.USART_HWFlowCtl > USART_HW_FLOW_CTL_RTS_CTS)
        return STATUS_INVALID_PARAM;

    if (config.USART_Oversampling < USART_OVERSAMPLING_16 ||
        config.USART_Oversampling > USART_OVERSAMPLING_8)
        return STATUS_INVALID_PARAM;

    // validate UART4/UART5 restrictions
    if (USART_IsUART(pUSARTHandle->pUSARTx))
    {
        if (config.USART_HWFlowCtl != USART_HW_FLOW_CTL_NONE)
            return STATUS_INVALID_PARAM;
        
        if (config.USART_NumStopBits != USART_STOP_BITS_1 &&
            config.USART_NumStopBits != USART_STOP_BITS_2)
            return STATUS_INVALID_PARAM;
    }

    USART_reg_t *pUSARTx = pUSARTHandle->pUSARTx;

    // =========== Enable peripheral clock ===========
    USART_PeriClkCtl(pUSARTx, ENABLE);

    // =========== Busy check ===========
    // TODO: check if busy and return STATUS_BUSY

    // =========== Disable interrupts ===========
    USART_DisableInterrupts(pUSARTx);

    // =========== Disable peripheral ===========
    USART_PeripheralControl(pUSARTx, DISABLE);

    // =========== Configure mode ===========

    // enable transmitter
    pUSARTx->CR1 |= (1U << USART_CR1_TE_BIT);
    // enable receiver
    pUSARTx->CR1 |= (1U << USART_CR1_RE_BIT);

    // disable receiver
    if (config.USART_Mode == USART_MODE_TX_ONLY)
        pUSARTx->CR1 &= ~(1U << USART_CR1_RE_BIT);

    // disable transmitter
    else if (config.USART_Mode == USART_MODE_RX_ONLY)
        pUSARTx->CR1 &= ~(1U << USART_CR1_TE_BIT);

    // =========== configure stop-bits ===========
    // clear current bit-field
    pUSARTx->CR2 &= ~(0x3U << USART_CR2_STOP_BIT);

    // set new bit-field
    pUSARTx->CR2 |= ((uint32_t)config.USART_NumStopBits << USART_CR2_STOP_BIT);

    // =========== Configure word-length ===========
    // clear current M-bit
    pUSARTx->CR1 &= ~(1U << USART_CR1_M_BIT);

    // set configured wordLen
    pUSARTx->CR1 |= (config.USART_WordLen << USART_CR1_M_BIT);

    // =========== Configure Parity ===========
    if (config.USART_ParityCtl == USART_PARITY_DISABLED)
    {
        // disable parity
        pUSARTx->CR1 &= ~(1U << USART_CR1_PCE_BIT);
    }
    else
    {
        // enable parity
        pUSARTx->CR1 |= (1U << USART_CR1_PCE_BIT);

        // parity selection
        if (config.USART_ParityCtl == USART_PARITY_EVEN)
            pUSARTx->CR1 &= ~(1U << USART_CR1_PS_BIT);
        else
            pUSARTx->CR1 |= (1U << USART_CR1_PS_BIT);
    }

    // =========== Configure HW Flow-control ===========
    // clear current configuration
    pUSARTx->CR3 &= ~((1U << USART_CR3_CTSE_BIT) | (1U << USART_CR3_RTSE_BIT));

    // enable CTS
    if (config.USART_HWFlowCtl == USART_HW_FLOW_CTL_CTS ||
        config.USART_HWFlowCtl == USART_HW_FLOW_CTL_RTS_CTS)
        pUSARTx->CR3 |= (1U << USART_CR3_CTSE_BIT);

    // enable RTS
    if (config.USART_HWFlowCtl == USART_HW_FLOW_CTL_RTS ||
        config.USART_HWFlowCtl == USART_HW_FLOW_CTL_RTS_CTS)
        pUSARTx->CR3 |= (1U << USART_CR3_RTSE_BIT);

    // =========== configure oversampling ===========
    if (config.USART_Oversampling == USART_OVERSAMPLING_16)
        pUSARTx->CR1 &= ~(1U << USART_CR1_OVER8_BIT);
    else
        pUSARTx->CR1 |= (1U << USART_CR1_OVER8_BIT);

    // =========== configure baud ===========
    status_t status = USART_SetBaudRate(pUSARTHandle);
    if (status != STATUS_OK)
    {
        USART_Reset(pUSARTx);
        return status;
    }

    // =========== Initialize runtime handle state ===========
    // TODO

    // =========== enable peripheral ===========
    USART_PeripheralControl(pUSARTx, ENABLE);
        
    return STATUS_OK;
}

/**************************************************************************
 * @fn		USART_SendData
 *
 * @brief	Blocking USART transmit.
 *
 * @details
 * Waits for TxE before writing each data word, then waits for TC before
 * returning. Data width is determined by word-length and parity control.
 *
 * @param[in] pUSARTHandle	Pointer to USART handle.
 * @param[in] pTxBuffer		Pointer to Tx buffer.
 * @param[in] len	        Number of data words to send.
 *
 * @return	STATUS_OK: All data words were sent and TC was set.
 *          STATUS_ERROR: The peripheral or transmitter was disabled.
 *          STATUS_INVALID_PARAM: An invalid argument was passed.
 *          STATUS_TIMEOUT: A TXE or TC polling budget was exhausted.
 *
 * @note	The len argument counts data words, not necessarily bytes of
 *          data. If sending 7-bits of data per word, the MSB is discarded
 *          from each byte. If sending 9-bits of data per word, the
 *          upper 7-bits of each 2-bytes is discarded.
 * 
 * @note    A timeout may occur after a partial transmission.
 ***************************************************************************/
status_t USART_SendData(USART_handle_t *pUSARTHandle, uint8_t *pTxBuffer, uint32_t len)
{
    // validate args
    if (!pTxBuffer ||
        (len == 0) ||
        !pUSARTHandle ||
        !pUSARTHandle->pUSARTx ||
        !USART_IsValidInstance(pUSARTHandle->pUSARTx)
        )
    {
        return STATUS_INVALID_PARAM;
    }

    USART_reg_t *pUSARTx = pUSARTHandle->pUSARTx;

    // ensure peripheral is enabled
    if ( (pUSARTx->CR1 & (1U << USART_CR1_UE_BIT)) == 0U)
        return STATUS_ERROR;
    
    // ensure transmitter is enabled
    if ( (pUSARTx->CR1 & (1U << USART_CR1_TE_BIT)) == 0U)
        return STATUS_ERROR;

    // extract M-bit (0 == 8-bit frame, 1 == 9-bit frame)
    uint8_t M = (pUSARTx->CR1 >> USART_CR1_M_BIT) &0x1U;

    // extract parity control (0 == disabled, 1 == enabled)
    uint8_t parity = (pUSARTx->CR1 >> USART_CR1_PCE_BIT) &0x1U;
    
    for (uint32_t i = 0; i < len; i++)
    {
        // wait for TxE
        status_t status = USART_WaitForFlag(pUSARTx, USART_FLAG_TXE, SET);
        if (status != STATUS_OK)
            return status;
        
        uint32_t data;
        if (M && !parity) // 9 data-bits
        {
            data =  (uint32_t)pTxBuffer[0] |
                    ((uint32_t) pTxBuffer[1] << 8U);
            
            // mask lowest 9-bits of data
            data &= 0x1FFU;
            pTxBuffer += 2;
        }
        else
        {
            data = *pTxBuffer;

            if (!M && parity)
                data &= 0x7FU;

            pTxBuffer++;
        }
        pUSARTx->DR = data;
    }

    // wait for transmission complete
    return USART_WaitForFlag(pUSARTx, USART_FLAG_TC, SET);
}

/**************************************************************************
 * @fn		USART_ReceiveData
 *
 * @brief	USART/UART blocking receive.
 *
 * @details
 * Waits for RxNE before reading each data word. Performs error-aware
 * polling. Payload width is determined by word-length and parity config.
 *
 * @param[in] pUSARTHandle	Pointer to USART handle.
 * @param[in] pTxBuffer		Pointer to Rx buffer.
 * @param[in] len	        Number of data words to receive.
 *
 * @return	STATUS_OK: All data words were successfully received.
 *          STATUS_ERROR: Peripheral or receiver is disabled, or an
 *          error was detected.
 *          STATUS_INVALID_PARAM: An invalid argument was passed.
 *          STATUS_TIMEOUT: Polling budget exceeded on RxNE.
 *
 * @note	len counts data words, not bytes. Data words may be 7, 8, or 9
 *          bits each depending on word-length and parity configuration.
 *          The buffer must provide enough space for len data-words.
 ***************************************************************************/
status_t USART_ReceiveData(USART_handle_t *pUSARTHandle, uint8_t *pRxBuffer, uint32_t len)
{
    // validate args
    if (!pRxBuffer ||
        (len == 0) ||
        !pUSARTHandle ||
        !pUSARTHandle->pUSARTx ||
        !USART_IsValidInstance(pUSARTHandle->pUSARTx)
        )
    {
        return STATUS_INVALID_PARAM;
    }

    USART_reg_t *pUSARTx = pUSARTHandle->pUSARTx;

    // ensure peripheral is enabled
    if ( (pUSARTx->CR1 & (1U << USART_CR1_UE_BIT)) == 0U)
        return STATUS_ERROR;
    
    // ensure receiver is enabled
    if ( (pUSARTx->CR1 & (1U << USART_CR1_RE_BIT)) == 0U)
        return STATUS_ERROR;

    // extract M-bit (0 == 8-bit frame, 1 == 9-bit frame)
    uint8_t M = (pUSARTx->CR1 >> USART_CR1_M_BIT) &0x1U;

    // extract parity control (0 == disabled, 1 == enabled)
    uint8_t parity = (pUSARTx->CR1 >> USART_CR1_PCE_BIT) &0x1U;

    for (uint32_t i = 0; i < len; i++)
    {
        // wait for RxNE
        status_t status = USART_WaitForRxNE(pUSARTx);
        if (status != STATUS_OK)
        {
            if (status == STATUS_ERROR)
                (void) pUSARTx->DR; // clears error flag and discards pending word
            return status;
        }

        uint32_t data = pUSARTx->DR;
        if (M && !parity) // 9-bits per word
        {
            data &= 0x1FFU;

            *(pRxBuffer++) = (uint8_t)(data & 0xFFU);
            *(pRxBuffer++) = (uint8_t)((data >> 8U) & 0x1U);
        }
        else
        {
            // mask to 7-bits if necessary
            data = (!M && parity) ? (data & 0x7FU) : data;
            *pRxBuffer++ = (uint8_t)data;
        }
    }
    return STATUS_OK;
}

// ***************************************************************
// ********************** INTERRUPT HELPERS **********************
// ***************************************************************

/**************************************************************************
 * @fn		USART_DisableInterrupts
 *
 * @brief	Disables all USART interrupt sources.
 *
 * @param[in] pUSARTx   Pointer to USART base address.
 *
 * @return	None.
 *
 * @note	This function immediately returns if invalid pointer is passed.
 ***************************************************************************/
static void USART_DisableInterrupts(USART_reg_t *pUSARTx)
{
    if (!pUSARTx || !USART_IsValidInstance(pUSARTx))
        return;
    
    // disable CR1 interrupts
    pUSARTx->CR1&=~((1U << USART_CR1_PEIE_BIT)  |
                    (1U << USART_CR1_TXEIE_BIT) |
                    (1U << USART_CR1_TCIE_BIT)  |
                    (1U << USART_CR1_RXNEIE_BIT)|
                    (1U << USART_CR1_IDLEIE_BIT));
    
    // disable CR2 interrupts
    pUSARTx->CR2&=~(1U << USART_CR2_LBDIE_BIT);

    // disable CR3 interrupts
    pUSARTx->CR3&=~((1U << USART_CR3_CTSIE_BIT) |
                    (1U << USART_CR3_EIE_BIT));
}


// ***************************************************************
// **************** INTERRUPT API IMPLEMENTATIONS ****************
// ***************************************************************

__weak void USART_ApplicationEventCallback(USART_handle_t *pHandle, USART_event_t event)
{
    (void) pHandle;
    (void) event;
}
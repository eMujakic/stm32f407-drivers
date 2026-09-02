/*
 * stm32f407xx_spi_driver.c
 *
 *  Created on: Aug 7, 2026
 *      Author: ernad
 */

#include "stm32f407xx_spi_driver.h"

// Interrupt handler function prototypes
static void SPI_handle_txe_interrupt(SPI_handle_t *pHandle);
static void SPI_handle_rxne_interrupt(SPI_handle_t *pHandle);
static void SPI_handle_ovr_err_interrupt(SPI_handle_t *pHandle);

/**************************************************************************
 * @fn		SPI_IsValidInstance
 *
 * @brief	Used to determine whether arg is a valid SPI base addr.
 *
 * @param[in] pSPIx		SPI peripheral base address.
 *
 * @return	Boolean indicating validity of passed SPI base addr.
 ***************************************************************************/
static inline uint8_t SPI_IsValidInstance(SPI_reg_t *pSPIx)
{
    return 
    (
        (pSPIx == SPI1) || 
        (pSPIx == SPI2) || 
        (pSPIx == SPI3) || 
        (pSPIx == SPI4)
    );
}

/**************************************************************************
 * @fn		SPI_WaitForFlag
 *
 * @brief	A blocking wait for certain flag bit to be SET or RESET.
 *
 * @details
 * Polls the passed SPI flag and blocks until either the desired value is
 * read, or the software timeout expires.
 *
 * @param[in] pSPIx		SPI peripheral base address.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_INVALID_PARAM / STATUS_BUSY).
 *
 ***************************************************************************/
status_t SPI_WaitForFlag(SPI_reg_t *pSPIx, uint32_t flagBit, uint8_t val)
{
    // validate args
    if (!SPI_IsValidInstance(pSPIx) ||
        flagBit > SPI_SR_FRE_BIT    ||
        val > 1U)
    {
        return STATUS_INVALID_PARAM;
    }
        
    uint32_t timeout = SPI_TIMEOUT;
    while(SPI_GetFlag(pSPIx, flagBit) != val)
    {
        if (timeout == 0U) return STATUS_BUSY;
        timeout--;
    }

    return STATUS_OK;
}

/**************************************************************************
 * @fn		SPI_IsMaster
 *
 * @brief	Used to determine whether SPI peripheral is in master-mode.
 *
 * @param[in] pSPIx		SPI peripheral base address.
 *
 * @return	Boolean indicating if SPI is master (1) or not (0).
 *
 ***************************************************************************/
static inline uint8_t SPI_IsMaster(SPI_reg_t *pSPIx)
{
    if ( !SPI_IsValidInstance(pSPIx) ) return 0;

    return (uint8_t)((pSPIx->CR1 >> SPI_CR1_MSTR_BIT) & 1U);
}

/**************************************************************************
 * @fn		SPI_ClearOVR
 *
 * @brief	Clears overrun bit in SPI status register.
 *
 * @param[in] pSPIx		SPI peripheral base address.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_INVALID_PARAM).
 ***************************************************************************/
status_t SPI_ClearOVR(SPI_reg_t *pSPIx)
{
    // validate instance
    if ( !SPI_IsValidInstance(pSPIx) )
    {
        return STATUS_INVALID_PARAM;
    }

    volatile uint32_t temp;

    // clear OVR by reading DR then reading SR
    temp = pSPIx->DR;
    temp = pSPIx->SR;

    (void)temp;

    return STATUS_OK;
}

/**************************************************************************
 * @fn		SPI_GetFlag
 *
 * @brief	Used to read flag in a SPI's status register (SR).
 *
 * @param[in] pSPIx		SPI peripheral base address.
 * @param[in] flagBit	Specific flag-bit to read.
 *
 * @return	The value of pSPIx's SR at flagBit. 
 ***************************************************************************/
uint8_t SPI_GetFlag(SPI_reg_t *pSPIx, uint32_t flagBit)
{
    if (!SPI_IsValidInstance(pSPIx))    return 0U;
    if (flagBit > SPI_SR_FRE_BIT)       return 0U;

    return (uint8_t) ((pSPIx->SR >> flagBit) & 1U);
}

/**************************************************************************
 * @fn		SPI_IsInterruptPending
 *
 * @brief	Checks whether a given interrupt type is currently pending.
 *
 * @param[in] pSPIx		SPI peripheral base address.
 * @param[in] type		Interrupt type to check if pending.
 *
 * @return	Boolean indicating whether interrupt is
 *          pending (1) or not (0).
 *
 * @note	Returns 0 if the interrupt-type passed is invalid.
 ***************************************************************************/
static uint8_t SPI_IsInterruptPending(SPI_reg_t *pSPIx, SPI_interrupt_t type)
{
    if (!SPI_IsValidInstance(pSPIx))    return 0U;

    uint8_t flag    = 0;
    uint32_t ie     = 0;    // interrupt-enable

    switch ( type )
    {
        // check for TXE interrupt
        case SPI_INTERRUPT_TXE:
            flag    = SPI_GetFlag(pSPIx, SPI_SR_TXE_BIT);
            ie      = pSPIx->CR2 & (1U << SPI_CR2_TXEIE_BIT);
            break;
        // check for RXNE interrupt
        case SPI_INTERRUPT_RXNE:
            flag    = SPI_GetFlag(pSPIx, SPI_SR_RXNE_BIT);
            ie      = pSPIx->CR2 & (1U << SPI_CR2_RXNEIE_BIT);
            break;
        // check for ERR interrupt
        case SPI_INTERRUPT_ERR:
            flag    = SPI_GetFlag(pSPIx, SPI_SR_OVR_BIT);
            ie      = pSPIx->CR2 & (1U << SPI_CR2_ERRIE_BIT);
            break;

        default:
            return 0U;
    }

    return (uint8_t)( flag && ie );
}

/**************************************************************************
 * @fn		SPI_WriteDR
 *
 * @brief	Helper function that writes 8/16 bit data to SPI DR.
 *
 * @param[in] pSPIx		SPI peripheral base address.
 * @param[in] data		Data to be written.
 *
 * @return	None.
 *
 * @note	Doesnt check if TX is empty, user must do so explicitly. 
 ***************************************************************************/
static void inline SPI_WriteDR(SPI_reg_t *pSPIx, uint16_t data)
{

    // 0 = SPI_DFF_8, 1 = SPI_DFF_16
    uint8_t dff = (pSPIx->CR1 >> SPI_CR1_DFF_BIT) & 1U;

    if (dff == 0U)
    {
        *((volatile uint8_t *)&pSPIx->DR) = (uint8_t) data;
    }
    else
    {
        *((volatile uint16_t *)&pSPIx->DR) = data;
    }
}

/**************************************************************************
 * @fn		SPI_WriteDummy
 *
 * @brief	Writes dummy data to SPI TX register.
 *
 *
 * @param[in] pSPIx		SPI peripheral base address.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_ERROR / STATUS_INVALID_PARAM / STATUS_BUSY).
 *
 * @note	Uses the SPI_WaitForFlag API which can timeout, if call
 *          does timeout, STATUS_BUSY is returned.
 ***************************************************************************/
static status_t SPI_WriteDummy(SPI_reg_t *pSPIx)
{
    if ( !SPI_IsValidInstance(pSPIx) )
        return STATUS_INVALID_PARAM;

    status_t timeout = SPI_WaitForFlag(pSPIx, SPI_SR_TXE_BIT, SET);
    if ( timeout )
        return STATUS_BUSY;

    SPI_WriteDR(pSPIx, 0xFFFFU);
    return STATUS_OK;
}

/**************************************************************************
 * @fn		SPI_PeriClkCtl
 *
 * @brief	Enables the peripheral clock for a given SPI peripheral.
 *
 * @param[in] pSPIx		Pointer to SPI's register base address.
 * @param[in] enable	Boolean saying if clk should be enabled or disabled.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_ERROR / STATUS_INVALID_PARAM).
 *
 * @note	This function is automatically called in SPI_Init(), so the user
 *          typically doesn't call this function explicitly.
 ***************************************************************************/
status_t SPI_PeriClkCtl(SPI_reg_t *pSPIx, uint8_t enable)
{
    if (!SPI_IsValidInstance(pSPIx)) return STATUS_INVALID_PARAM;
    if (enable != ENABLE && enable != DISABLE) return STATUS_INVALID_PARAM;

    if (pSPIx == SPI1)
    {
        if (enable == ENABLE)   SPI1_PCLK_EN();
        else                    SPI1_PCLK_DI();
    }

    else if (pSPIx == SPI2)
    {
        if (enable == ENABLE)   SPI2_PCLK_EN();
        else                    SPI2_PCLK_DI();
    }

    else if (pSPIx == SPI3)
    {
        if (enable == ENABLE)   SPI3_PCLK_EN();
        else                    SPI3_PCLK_DI();
    }

    else if (pSPIx == SPI4)
    {
        if (enable == ENABLE)   SPI4_PCLK_EN();
        else                    SPI4_PCLK_DI();
    }

    else return STATUS_INVALID_PARAM;

    return STATUS_OK;
}

/**************************************************************************
 * @fn		SPI_ConfigInit
 *
 * @brief	Initializes a given SPI config struct to safe defaults.
 *
 * @param[in] pConfig	Pointer to SPI_config_t struct to be initialized.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_ERROR / STATUS_INVALID_PARAM).
 *
 * @note	This function is primarily for QOL, so the user doesn't need to
 *          explicitly set every struct value before calling SPI_Init().
 ***************************************************************************/
status_t SPI_ConfigInit(SPI_config_t *pConfig)
{
    if(!pConfig) return STATUS_INVALID_PARAM;

    pConfig->SPI_DeviceMode     = SPI_MODE_SLAVE;
    pConfig->SPI_BusConfig      = SPI_BUS_FULL_DUPLEX;
    pConfig->SPI_BaudPrescaler  = SPI_BAUD_DIV2;
    pConfig->SPI_DFF            = SPI_DFF_8;
    pConfig->SPI_CPOL           = SPI_CPOL_LOW;
    pConfig->SPI_CPHA           = SPI_CPHA_FIRST_EDGE;
    pConfig->SPI_SSM            = SPI_SSM_DI;
    pConfig->SPI_BitOrder       = SPI_MSB_FIRST;

    return STATUS_OK;
}

/**************************************************************************
 * @fn		SPI_Init
 *
 * @brief	This function initializes a SPI peripheral according
 *          to config settings
 *
 * @details
 * This function validates inputs, then uses bitmasks to set a temporary
 * register value to the desired values for the SPI's CR1 register. It then
 * enables the SPI's peripheral clock, and writes to its CR1 register.
 *
 * @param[in] pSPIHandle	SPI handle for the peripheral to be initialized.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_ERROR / STATUS_INVALID_PARAM).
 *
 * @note	This function automatically sets the SSI bit if the desired
 *          configuration is in master mode using SSM to avoid mode-faults.
 ***************************************************************************/
status_t	SPI_Init(SPI_handle_t *pSPIHandle)
{
    // ================ Validate Inputs ================
    if (!pSPIHandle || !pSPIHandle->pSPIx)
        return STATUS_INVALID_PARAM;

    if (!SPI_IsValidInstance(pSPIHandle->pSPIx))
        return STATUS_INVALID_PARAM;


    uint32_t tempreg = 0U;
    SPI_config_t config = pSPIHandle->SPI_Config;

    if(config.SPI_DeviceMode > SPI_MODE_MASTER)
        return STATUS_INVALID_PARAM;

    if(config.SPI_BusConfig > SPI_BUS_SIMPLEX_RX)
        return STATUS_INVALID_PARAM;

    if(config.SPI_BaudPrescaler > SPI_BAUD_DIV256)
        return STATUS_INVALID_PARAM;

    if(config.SPI_DFF > SPI_DFF_16)
        return STATUS_INVALID_PARAM;

    if(config.SPI_CPOL > SPI_CPOL_HIGH)
        return STATUS_INVALID_PARAM;

    if(config.SPI_CPHA > SPI_CPHA_SECOND_EDGE)
        return STATUS_INVALID_PARAM;

    if(config.SPI_SSM > SPI_SSM_EN)
        return STATUS_INVALID_PARAM;

    if(config.SPI_BitOrder > SPI_LSB_FIRST)
        return STATUS_INVALID_PARAM;
    

    // ================ Configure Device Mode ================
    if(config.SPI_DeviceMode == SPI_MODE_MASTER)
        tempreg |= (1U << SPI_CR1_MSTR_BIT);

    // ================ Configure Bus ================

    if(config.SPI_BusConfig == SPI_BUS_FULL_DUPLEX)
        tempreg &= ~(1U << SPI_CR1_BIDIMODE_BIT);
    else if (config.SPI_BusConfig == SPI_BUS_HALF_DUPLEX_TX)
    {
        tempreg |= (1U << SPI_CR1_BIDIMODE_BIT);
        tempreg |= (1U << SPI_CR1_BIDIOE_BIT);
    }
    else if (config.SPI_BusConfig == SPI_BUS_HALF_DUPLEX_RX)
    {
        tempreg |= (1U << SPI_CR1_BIDIMODE_BIT);
        tempreg &= ~(1U << SPI_CR1_BIDIOE_BIT);
    }
    else if(config.SPI_BusConfig == SPI_BUS_SIMPLEX_RX)
    {
        tempreg &= ~(1U << SPI_CR1_BIDIMODE_BIT);
        tempreg |= (1U << SPI_CR1_RXONLY_BIT);
    }

    // ================ Configure Prescaler ================
    tempreg |= (config.SPI_BaudPrescaler << SPI_CR1_BR_BIT);

    // ================ Configure DFF ================
    tempreg |= (config.SPI_DFF << SPI_CR1_DFF_BIT);

    // ================ Configure CPOL ================
    tempreg |= (config.SPI_CPOL << SPI_CR1_CPOL_BIT);

    // ================ Configure CPHA ================
    tempreg |= (config.SPI_CPHA << SPI_CR1_CPHA_BIT);

    // ================ Configure Bit Order ================
    tempreg |= (config.SPI_BitOrder << SPI_CR1_LSBFIRST_BIT);

    // ================ Configure SSM ================
    tempreg |= (config.SPI_SSM << SPI_CR1_SSM_BIT);

    // sets internal NSS high when using software slave management as master
    if (config.SPI_DeviceMode == SPI_MODE_MASTER && config.SPI_SSM == SPI_SSM_EN)
        tempreg |= (1U << SPI_CR1_SSI_BIT);

    SPI_PeriClkCtl(pSPIHandle->pSPIx, ENABLE);  // enable clock in init function
    pSPIHandle->pSPIx->CR1 = tempreg;

    return STATUS_OK;
}

/**************************************************************************
 * @fn		SPI_Reset
 *
 * @brief	Sets and resets the SPI's RSTR bit to reset the peripheral.
 *
 * @param[in] pSPIx	    Pointer to SPI's base address.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_ERROR / STATUS_INVALID_PARAM).
 ***************************************************************************/
status_t SPI_Reset(SPI_reg_t *pSPIx)
{
    if (!SPI_IsValidInstance(pSPIx)) return STATUS_INVALID_PARAM;

    if      (pSPIx == SPI1) SPI1_RESET();
    else if (pSPIx == SPI2) SPI2_RESET();
    else if (pSPIx == SPI3) SPI3_RESET();
    else if (pSPIx == SPI4) SPI4_RESET();
    else                    return STATUS_INVALID_PARAM;
        
    return STATUS_OK;
}

/**************************************************************************
 * @fn		SPI_Send
 *
 * @brief	Sends data using SPI.
 *
 * @param[in] pSPIx		    Pointer to SPI's base address.
 * @param[in] pTXBuffer		Buffer containing data to be sent.
 * @param[in] len		    The amount of bytes to send.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_ERROR / STATUS_INVALID_PARAM).
 *
 * @note    This is a blocking send-data.
 * @note    If using 16-bit data-frame format, length must be even.
 ***************************************************************************/
status_t    SPI_Send(SPI_reg_t *pSPIx, uint8_t *pTXBuffer, uint32_t len)
{
    if (!SPI_IsValidInstance(pSPIx) || !pTXBuffer)
    {
        return STATUS_INVALID_PARAM;
    }

    uint8_t dff = (pSPIx->CR1 >> SPI_CR1_DFF_BIT) & 1U;

    // Ensures len is even if dff is 2-bytes wide
    if (dff && (len % 2)) return STATUS_INVALID_PARAM;  
    
    while(len)
    {
        while ( !SPI_GetFlag(pSPIx, SPI_SR_TXE_BIT) );    // poll until tx-empty

        if(!dff)
        {
            *((volatile uint8_t*)&pSPIx->DR) = *pTXBuffer;

            pTXBuffer++;
            len--;
        }
        else
        {
            uint16_t data = ((uint16_t) pTXBuffer[1] << 8U | pTXBuffer[0] );
            *((volatile uint16_t*)&pSPIx->DR) = data;

            pTXBuffer += 2;
            len -= 2;
        }

        // Clear RX buffer
        while ( !SPI_GetFlag(pSPIx, SPI_SR_RXNE_BIT) );

        if (!dff)
            (void)*(volatile uint8_t *)&pSPIx->DR;
        else
            (void)*(volatile uint16_t *)&pSPIx->DR;
    }

    while(!SPI_GetFlag(pSPIx, SPI_SR_TXE_BIT)); // wait until TX is empty
    while(SPI_GetFlag(pSPIx, SPI_SR_BSY_BIT));  // wait until not busy
    
    return STATUS_OK;
}

/**************************************************************************
 * @fn		SPI_Receive
 *
 * @brief	An API which sends data via SPI.
 *
 * @param[in] pSPIx     Pointer to SPI's base address.
 * @param[in] pRXBuffer A data buffer to write received data to.
 * @param[in] len       The number of bytes to receive.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_ERROR / STATUS_INVALID_PARAM).
 *
 * @note	Sends dummy data if the peripheral is in master mode.
 * @note    If using 16-bit data-frame format, length must be even.
 ***************************************************************************/
status_t SPI_Receive(SPI_reg_t *pSPIx, uint8_t *pRXBuffer, uint32_t len)
{
    if (!SPI_IsValidInstance(pSPIx) || !pRXBuffer)
    {
        return STATUS_INVALID_PARAM;
    }

    uint8_t dff = (pSPIx->CR1 >> SPI_CR1_DFF_BIT) & 1U;

    // Ensures len is even if dff is 2-bytes wide
    if (dff && (len % 2)) return STATUS_INVALID_PARAM;
    
    while(len)
    {
        // Fill TX buffer with dummy bytes if in Master mode
        if ( SPI_IsMaster(pSPIx) )
        {
            SPI_WriteDummy(pSPIx);
        }

        while ( !SPI_GetFlag(pSPIx, SPI_SR_RXNE_BIT) );     // poll until rx-not-empty

        if(!dff)
        {
            // read one byte from DR
            *pRXBuffer = *((volatile uint8_t*)&pSPIx->DR);

            pRXBuffer++;
            len--;
        }
        else
        {
            // read two bytes from DR
            *((uint16_t*)pRXBuffer) = *((volatile uint16_t*)&pSPIx->DR);

            pRXBuffer += 2;
            len -= 2;
        }
    }

    return STATUS_OK;
}

/**************************************************************************
 * @fn		SPI_PeripheralControl
 *
 * @brief	Used to set/clear SPE bit of SPI peripheral.
 *
 * @param[in] pSPIx		Pointer to SPI's base address.
 * @param[in] enable	Boolean saying whether SPI should be enabled or disabled.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_ERROR / STATUS_INVALID_PARAM).
 ***************************************************************************/
 status_t SPI_PeripheralControl(SPI_reg_t *pSPIx, uint8_t enable)
 {
    if (!SPI_IsValidInstance(pSPIx))
        return STATUS_INVALID_PARAM;

    if (enable == ENABLE)
    {
        pSPIx->CR1 |= (1U << SPI_CR1_SPE_BIT);
    }
    else if (enable == DISABLE)
    {
        // while( SPI_GetFlag(pSPIx, SPI_SR_BSY_BIT) );    // wait until not busy
        pSPIx->CR1 &= ~(1U << SPI_CR1_SPE_BIT);
    }
    else
        return STATUS_INVALID_PARAM;

    return STATUS_OK;
}

/**************************************************************************
 * @fn		SPI_SSOEConfig
 *
 * @brief	Used to set/clear SSOE bit of SPI peripheral.
 *
 * @param[in] pSPIx		Pointer to SPI's base address.
 * @param[in] enable	Boolean saying whether SPI should be enabled or disabled.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_ERROR / STATUS_INVALID_PARAM).
 *
 * @note    
 ***************************************************************************/
 status_t SPI_SSOEConfig(SPI_reg_t *pSPIx, uint8_t enable)
 {
    if (!SPI_IsValidInstance(pSPIx))
        return STATUS_INVALID_PARAM;

    if (enable == ENABLE)
        pSPIx->CR2 |= (1U << SPI_CR2_SSOE_BIT);
    else if (enable == DISABLE)
        pSPIx->CR2 &= ~(1U << SPI_CR2_SSOE_BIT);
    else
        return STATUS_INVALID_PARAM;

    return STATUS_OK;
}

/**************************************************************************
 * @fn		SPI_SendIT
 *
 * @brief	Setups/starts interrupt-driven SPI data transmission.
 *
 * @details
 * Saves necessary interrupt state in SPI handle, sets SPI as busy-in-TX 
 * in handle, and enables TXEIE interrupts for the SPI peripheral.
 *
 * @param[in] pHandle       Pointer to SPI handle
 * @param[in] pTXBuffer     Pointer to data buffer to send.
 * @param[in] len           Number of bytes to send.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_INVALID_PARAM / STATUS_BUSY).
 *
 * @note	Doesn't actually send any data, simply sets up interrupt state
 *          necessary for interrupt driven transmission.
 ***************************************************************************/
status_t SPI_SendIT(SPI_handle_t *pHandle, uint8_t *pTXBuffer, uint32_t len)
{
    // validate args
    if (!pHandle || !pTXBuffer || !len ||
        !SPI_IsValidInstance(pHandle->pSPIx))
    {
        return STATUS_INVALID_PARAM;
    }
        
    // return if SPI is busy
    if ( pHandle->TxState == SPI_BUSY_TX )
    {
        return STATUS_BUSY;
    }

    // ensure len is multiple of 2 if dff=16-bit
    uint8_t dff = (pHandle->pSPIx->CR1 >> SPI_CR1_DFF_BIT) & 1U;
    if ( dff && (len % 2U) )
        return STATUS_INVALID_PARAM;

    // save Tx-buffer and length info in handle
    pHandle->pTxBuffer  = pTXBuffer;
    pHandle->TxLen      = len;

    // mark SPI state as busy
    pHandle->TxState = SPI_BUSY_TX;

    // enable TXEIE control-bit to trigger interrupt when TXE is set in SR
    pHandle->pSPIx->CR2 |= (1U << SPI_CR2_TXEIE_BIT);

    return STATUS_OK;
}

 /**************************************************************************
 * @fn		SPI_ReceiveIT
 *
 * @brief	Setups/starts interrupt-driven SPI data receive.
 *
 * @details
 * Saves necessary interrupt state in SPI handle, sets SPI as busy in RX,
 * and enables RXNE interrupts for given SPI peripheral.
 *
 * @param[in] pHandle       Pointer to SPI handle
 * @param[in] pRXBuffer     Pointer to buffer to receive data into.
 * @param[in] len           Number of bytes to read.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_INVALID_PARAM / STATUS_BUSY).
 *
 * @note	Doesn't actually read any data, simply sets up necesssary
 *          interrupt state to perform interrupt-driven receives.
 ***************************************************************************/
 status_t SPI_ReceiveIT(SPI_handle_t *pHandle, uint8_t *pRXBuffer, uint32_t len)
{
    // validate args
    if (!pHandle || !pRXBuffer || !len ||
        !SPI_IsValidInstance(pHandle->pSPIx))
    {
        return STATUS_INVALID_PARAM;
    }
        
    // return if SPI is busy
    if ( pHandle->RxState == SPI_BUSY_RX )
        return STATUS_BUSY;

    // ensure len is multiple of 2 if dff=16-bit
    uint8_t dff = (pHandle->pSPIx->CR1 >> SPI_CR1_DFF_BIT) & 1U;
    if ( dff && (len % 2U) )
        return STATUS_INVALID_PARAM;

    // save Rx-buffer and length info in handle
    pHandle->pRxBuffer  = pRXBuffer;
    pHandle->RxLen      = len;

    // mark SPI state as busy
    pHandle->RxState = SPI_BUSY_RX;

    // enable RXNEIE control-bit to trigger interrupt when RXNE is set in SR
    pHandle->pSPIx->CR2 |= (1U << SPI_CR2_RXNEIE_BIT);

    // Initially fill TX buffer with dummy data if in Master mode
    if ( SPI_IsMaster((pHandle->pSPIx)) )
        SPI_WriteDummy(pHandle->pSPIx);

    return STATUS_OK;
}

/**************************************************************************
 * @fn		SPI_IRQHandling
 *
 * @brief	Dispatches pending SPI interrupt events to their handlers.
 *
 * @details
 * This is typically the function called by the actual SPI ISR. It 
 * dispatches pending SPI interrupt events to their corresponding
 * handlers.
 *
 * @param[in] pHandle       Pointer to SPI handle.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_INVALID_PARAM).
 *
 * @note    First handles RXNE interrupts, then TXE, then ERR.
 * @note    RXNE is handled first to reduce OVR error risk.
 * @note	Currently only handles OVR errors.
 ***************************************************************************/
status_t SPI_IRQHandling(SPI_handle_t *pHandle)
{
    // validate handle & instance
    if (!pHandle ||
        !SPI_IsValidInstance(pHandle->pSPIx))
    {
        return STATUS_INVALID_PARAM;
    }

    SPI_interrupt_t interrupt = SPI_INTERRUPT_INVALID;

    // handle rxne interrupt FIRST to avoid overrun
    interrupt = SPI_IsInterruptPending(pHandle->pSPIx, SPI_INTERRUPT_RXNE);
    if ( interrupt == SPI_INTERRUPT_RXNE )
        SPI_handle_rxne_interrupt(pHandle);

    // handle txe interrupt
    interrupt = SPI_IsInterruptPending(pHandle->pSPIx, SPI_INTERRUPT_TXE);
    if ( interrupt == SPI_INTERRUPT_TXE )
        SPI_handle_txe_interrupt(pHandle);

    // handle err interrupt
    interrupt = SPI_IsInterruptPending(pHandle->pSPIx, SPI_INTERRUPT_ERR);
    if ( interrupt == SPI_INTERRUPT_ERR )
        SPI_handle_ovr_err_interrupt(pHandle);

    return STATUS_OK;
}

/**************************************************************************
 * @fn		SPI_CloseTransmission
 *
 * @brief	Disables TXE interrupts and clears interrupt state in handle.
 *
 * @param[in] pHandle       Pointer to SPI handle.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_INVALID_PARAM).
 *
 ***************************************************************************/
status_t SPI_CloseTransmission(SPI_handle_t *pHandle)
{
    // validate handle & instance
    if (!pHandle ||
        !SPI_IsValidInstance(pHandle->pSPIx))
    {
        return STATUS_INVALID_PARAM;
    }

    pHandle->pSPIx->CR2 &= ~(1U << SPI_CR2_TXEIE_BIT);   // clear TXEIE bit
    pHandle->pTxBuffer  = NULL;
    pHandle->TxLen      = 0U;
    pHandle->TxState    = SPI_READY;                    // resets SPI TX interrupt state

    return STATUS_OK;
}

/**************************************************************************
 * @fn		SPI_CloseReception
 *
 * @brief	Disables RXNE interrupts and clears interrupt state in handle.
 *
 * @param[in] pHandle       Pointer to SPI handle.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_INVALID_PARAM).
 ***************************************************************************/
status_t SPI_CloseReception(SPI_handle_t *pHandle)
{
    // validate handle & instance
    if (!pHandle ||
        !SPI_IsValidInstance(pHandle->pSPIx))
    {
        return STATUS_INVALID_PARAM;
    }

    pHandle->pSPIx->CR2 &= ~(1U << SPI_CR2_RXNEIE_BIT);   // clear RXNEIE bit
    pHandle->pRxBuffer  = NULL;
    pHandle->RxLen      = 0;
    pHandle->RxState    = SPI_READY;            // resets SPI RX interrupt state

    return STATUS_OK;
}

/**************************************************************************
 * @fn		SPI_handle_txe_interrupt
 *
 * @brief	TXE interrupt handler called by SPI_IRQHandling().
 *
 * @details
 * Handles TXE interrupts by writing one byte to DR and discarding
 * the received byte. When all bytes are sent, the transmission is 
 * closed via SPI_CloseTransmission() and the application callback
 * is invoked with the SPI_EVENT_TX_DONE event.
 *
 * @param[in] pHandle       Pointer to SPI handle.
 *
 * @return	None.
 *
 * @note	In 16-bit DFF mode, TxLen must be even. This should
 *          be validated before enabling TXE interrupt.
 *
 * @note    This API returns when the final byte is written to DR,
 *          not after its done transmitting. The user may want to
 *          wait until SPI is not busy via SPI_WaitForFlag.
 ***************************************************************************/
static void SPI_handle_txe_interrupt(SPI_handle_t *pHandle)
{
    // validating pointers
    if (!pHandle ||
        !SPI_IsValidInstance(pHandle->pSPIx))
    {
        return;
    }

    SPI_reg_t *pSPIx    = pHandle->pSPIx;

    // data-frame format, (1 = 16-bit, 0 = 8-bit)
    uint8_t dff         = (pSPIx->CR1 >> SPI_CR1_DFF_BIT) & 1U;

    if ( pHandle->TxLen )
    {
        // handle 8-bit dff
        if(!dff)
        {
            *((volatile uint8_t*)&pSPIx->DR) = *(pHandle->pTxBuffer);

            pHandle->pTxBuffer++;
            pHandle->TxLen--;
        }
        // handle 16-bit dff
        else
        {
            uint16_t data = ((uint16_t) (pHandle->pTxBuffer)[1] << 8U | (pHandle->pTxBuffer)[0] );
            *((volatile uint16_t*)&pSPIx->DR) = data;

            pHandle->pTxBuffer += 2;
            pHandle->TxLen -= 2;
        }

        // Discard received byte
        if ( SPI_GetFlag(pSPIx, SPI_SR_RXNE_BIT) )
        {
            if  (!dff)
                (void)*(volatile uint8_t *)&pSPIx->DR;
            else
                (void)*(volatile uint16_t *)&pSPIx->DR;
        }
    }

    // close SPI transmission and update state

    // returns when the final byte is written to DR,
    // not after its done transmitting.
    if ( !pHandle->TxLen )
    {
        SPI_CloseTransmission(pHandle);
        SPI_ApplicationEventCallback(pHandle, SPI_EVENT_TX_DONE);
    }
}

/**************************************************************************
 * @fn		SPI_handle_rxne_interrupt
 *
 * @brief	RXNE interrupt handler called by SPI_IRQHandling().
 *
 * @details
 * Handles RXNE interrupts by reading data into pHandle->pRxBuffer from
 * the DR according to the DFF. After each read, if the peripheral is in
 * master mode and there are more bytes to read, dummy bytes are written
 * into the DR to ensure clock is being generated.
 *
 * When all requested data has been received, reception is closed via
 * SPI_CloseReception() and the application callback is invoked with
 * the SPI_EVENT_RX_DONE event. 
 *
 * @param[in] pHandle       Pointer to SPI handle.
 *
 * @return	None.
 *
 * @note	RxLen in the SPI handle must be divisible by DFF, otherwise
 *          OOB accesses may occur.
 ***************************************************************************/
static void SPI_handle_rxne_interrupt(SPI_handle_t *pHandle)
{
    // validating pointers
    if (!pHandle ||
        !SPI_IsValidInstance(pHandle->pSPIx))
    {
        return;
    }

    SPI_reg_t *pSPIx    = pHandle->pSPIx;

    // data-frame format, (1 = 16-bit, 0 = 8-bit)
    uint8_t dff         = (pSPIx->CR1 >> SPI_CR1_DFF_BIT) & 1U;

    if ( pHandle->RxLen )
    {
        if(!dff)
        {
            // read one byte from DR
            *(pHandle->pRxBuffer) = *((volatile uint8_t*)&pSPIx->DR);

            (pHandle->pRxBuffer)++;
            pHandle->RxLen--;
        }
        else
        {
            // read two bytes from DR
            *((uint16_t*)(pHandle->pRxBuffer)) = *((volatile uint16_t*)&pSPIx->DR);

            (pHandle->pRxBuffer) += 2;
            pHandle->RxLen -= 2;
        }
    }

    if ( !pHandle->RxLen )
    {
        SPI_CloseReception(pHandle);
        SPI_ApplicationEventCallback(pHandle, SPI_EVENT_RX_DONE);
    }
    else
    {
        // Fill TX buffer with dummy bytes if in Master mode
        if ( SPI_IsMaster(pSPIx) && SPI_GetFlag(pSPIx, SPI_SR_TXE_BIT) )
            SPI_WriteDummy(pSPIx);
    }
}

/**************************************************************************
 * @fn		SPI_handle_ovr_err_interrupt
 *
 * @brief	OVR error interrupt handler called by SPI_IRQHandling().
 *
 * @details
 * This handler handles OVR errors by calling the SPI_ClearOVR() API, which
 * clears the OVR flag in the peripheral's SR by reading DR, then reading
 * SR. After clearing the OVR error, the application callback is invoked
 * with the SPI_EVENT_OVR_ERR event.
 *
 * @param[in] pHandle       Pointer to SPI handle.
 *
 * @return	None.
 *
 * @note	The OVR is not cleared if the peripheral is busy transmitting
 *          at the time the handler is invoked, since reading DR may
 *          corrupt an active transfer.
 ***************************************************************************/
static void SPI_handle_ovr_err_interrupt(SPI_handle_t *pHandle)
{
    // validating pointers
    if (!pHandle ||
        !SPI_IsValidInstance(pHandle->pSPIx))
    {
        return;
    }

    SPI_reg_t *pSPIx    = pHandle->pSPIx;

    // clear ovr flag if not busy in TX
    if ( pHandle->TxState != SPI_BUSY_TX )
    {
        SPI_ClearOVR(pSPIx);
    }

    SPI_ApplicationEventCallback(pHandle, SPI_EVENT_OVR_ERR);
}

/**************************************************************************
 * @fn		SPI_ApplicationEventCallback
 *
 * @brief	Implemented by the user to handle certain interrupt events.
 *
 * @details
 * A weakly-implemented callback function which is invoked by the SPI
 * interrupt handlers to notify the user-application of certain events.
 *
 * @param[in] pHandle       Pointer to SPI handle.
 * @param[in] event		    The event that occurred to trigger the callback.
 *
 * @return	None.
 *
 * @note	This API is weakly-implemented, meaning the user should implement
 *          any desired functionality themselves.
 ***************************************************************************/
__weak void SPI_ApplicationEventCallback(SPI_handle_t *pHandle, SPI_event_t event)
{
    ;
}
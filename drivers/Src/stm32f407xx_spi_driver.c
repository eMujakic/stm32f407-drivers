/*
 * stm32f407xx_spi_driver.c
 *
 *  Created on: Aug 7, 2026
 *      Author: ernad
 */

#include "stm32f407xx_spi_driver.h"

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
 * @param[in] pSPIHandle	TODO
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
 * @details
 * TODO
 *
 * @param[in] pSPIx		    Pointer to SPI's base address.
 * @param[in] pTXBuffer		Buffer containing data to be sent.
 * @param[in] len		    The amount of bytes to send.
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_ERROR / STATUS_INVALID_PARAM).
 *
 * @note    This is a blocking send-data.
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

        // Drain RX buffer
        if(SPI_GetFlag(pSPIx, SPI_SR_RXNE_BIT))
        {
            if (!dff)
                (void)*(volatile uint8_t *)&pSPIx->DR;
            else
                (void)*(volatile uint16_t *)&pSPIx->DR;
        }
    }

    while(!SPI_GetFlag(pSPIx, SPI_SR_TXE_BIT)); // wait until TX is empty
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
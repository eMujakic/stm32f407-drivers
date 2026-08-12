/*
 * stm32f407xx_spi_driver.c
 *
 *  Created on: Aug 7, 2026
 *      Author: ernad
 */

#include "stm32f407xx_spi_driver.h"


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

static inline uint8_t SPI_GetFlag(SPI_reg_t *pSPIx, uint32_t flagBit)
{
    if (!SPI_IsValidInstance(pSPIx))    return 0U;
    if (flagBit > SPI_SR_FRE_BIT)       return 0U;

    return (uint8_t) ((pSPIx->SR >> flagBit) & 1U);
}

/**************************************************************************
 * @fn		SPI_PeriClkCtl
 *
 * @brief	TODO
 *
 * @details
 * TODO
 *
 * @param[in] TODO		TODO
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_ERROR / STATUS_INVALID_PARAM).
 *
 * @note	TODO
 ***************************************************************************/
status_t	SPI_PeriClkCtl(SPI_reg_t *pSPIx, uint8_t enable)
{
    if (!SPI_IsValidInstance(pSPIx)) return STATUS_INVALID_PARAM;
    if (enable != ENABLE && enable != DISABLE) return STATUS_INVALID_PARAM;

    if (pSPIx == SPI1)
    {
        if (enable == ENABLE)   SPI1_PCLK_EN();
        else                    SPI1_PCLK_DI();
    }

    if (pSPIx == SPI2)
    {
        if (enable == ENABLE)   SPI2_PCLK_EN();
        else                    SPI2_PCLK_DI();
    }

    if (pSPIx == SPI3)
    {
        if (enable == ENABLE)   SPI3_PCLK_EN();
        else                    SPI3_PCLK_DI();
    }

    if (pSPIx == SPI4)
    {
        if (enable == ENABLE)   SPI4_PCLK_EN();
        else                    SPI4_PCLK_DI();
    }

    return STATUS_ERROR;
}

/**************************************************************************
 * @fn		SPI_ConfigInit
 *
 * @brief	Initializes a given SPI config struct to safe defaults.
 *
 * @details
 * TODO
 *
 * @param[in] TODO		TODO
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_ERROR / STATUS_INVALID_PARAM).
 *
 * @note	TODO
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

    return STATUS_OK;
}

/**************************************************************************
 * @fn		SPI_Init
 *
 * @brief	TODO
 *
 * @details
 * TODO
 *
 * @param[in] TODO		TODO
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_ERROR / STATUS_INVALID_PARAM).
 *
 * @note	TODO
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
    

    // ================ Configure Device Mode ================
    if(config.SPI_DeviceMode == SPI_MODE_MASTER)
        tempreg |= (1U << SPI_CR1_MSTR_BIT);

    // ================ Configure Bus ================

    if(config.SPI_BusConfig == SPI_BUS_FULL_DUPLEX)
        tempreg &= ~(1U << SPI_CR1_BIDIMODE_BIT);
    else if (config.SPI_BusConfig == SPI_BUS_HALF_DUPLEX)
        tempreg |= (1U << SPI_CR1_BIDIMODE_BIT);
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

    // ================ Configure SSM ================
    tempreg |= (config.SPI_SSM << SPI_CR1_SSM_BIT);

    // sets internal NSS high when using software slave management as master
    if (config.SPI_DeviceMode == SPI_MODE_MASTER && config.SPI_SSM == SPI_SSM_EN)
        tempreg |= (1U << SPI_CR1_SSI_BIT);

    pSPIHandle->pSPIx->CR1 = tempreg;

    return STATUS_OK;
}

/**************************************************************************
 * @fn		SPI_Reset
 *
 * @brief	TODO
 *
 * @details
 * TODO
 *
 * @param[in] TODO		TODO
 *
 * @return	Status indicating whether the command was successful (STATUS_OK)
 *			or not (STATUS_ERROR / STATUS_INVALID_PARAM).
 *
 * @note	TODO
 ***************************************************************************/
status_t 	SPI_Reset(SPI_reg_t *pSPIx)
{
    if (!pSPIx) return STATUS_INVALID_PARAM;

    if(pSPIx == SPI1)
        SPI1_RESET();

    else if(pSPIx == SPI2)
        SPI2_RESET();

    else if(pSPIx == SPI3)
        SPI3_RESET();

    else if(pSPIx == SPI4)
        SPI4_RESET();

    else
        return STATUS_INVALID_PARAM;
    
    return STATUS_OK;
}

/**************************************************************************
 * @fn		TODO
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
 * @note    This is a blocking send-data.
 ***************************************************************************/
status_t    SPI_Send(SPI_reg_t *pSPIx, uint8_t *pTXBuffer, uint32_t len)
{
    if (!SPI_IsValidInstance(pSPIx) || !pTXBuffer)
    {
        return STATUS_INVALID_PARAM;
    }

    uint8_t DFF = ((pSPIx->CR1 & (1U << SPI_CR1_DFF_BIT)) >> SPI_CR1_DFF_BIT);
    if (DFF && (len % 2)) return STATUS_INVALID_PARAM;      // validate len is even
    
    while(len)
    {
        while ( !SPI_GetFlag(pSPIx, SPI_SR_TXE_BIT) );    // poll until tx-empty

        if(!DFF)
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
    }

    while(pSPIx->SR & SPI_SR_BSY_BIT);  // wait until SPI is done sending
    return STATUS_OK;
}
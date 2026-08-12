/*
 * stm32f407xx_spi_driver.h
 *
 *  Created on: Aug 7, 2026
 *      Author: ernad
 */

#ifndef INC_STM32F407XX_SPI_DRIVER_H_
#define INC_STM32F407XX_SPI_DRIVER_H_

#include "stm32f407xx.h"

// ====================== SPI device mode enum =====================
typedef enum
{
    SPI_MODE_SLAVE  = 0U,
    SPI_MODE_MASTER = 1U                // If device is in master mode, it is responsible for producing SCLK
} SPI_device_mode_t;

// ====================== SPI bus config enum ======================
typedef enum
{
    SPI_BUS_FULL_DUPLEX = 0U,
    SPI_BUS_HALF_DUPLEX = 1U,
    SPI_BUS_SIMPLEX_RX  = 2U            // simplex, RX-only
} SPI_bus_config_t;

// ====================== SPI sclk prescaler enum ==================
typedef enum
{
    SPI_BAUD_DIV2    = 0U,
    SPI_BAUD_DIV4    = 1U,
    SPI_BAUD_DIV8    = 2U,
    SPI_BAUD_DIV16   = 3U,
    SPI_BAUD_DIV32   = 4U,
    SPI_BAUD_DIV64   = 5U,
    SPI_BAUD_DIV128  = 6U,
    SPI_BAUD_DIV256  = 7U,
} SPI_baud_prescaler_t;

// ====================== SPI DFF enum =============================
typedef enum
{
    SPI_DFF_8   = 0U,                   // 8-bit data-frame
    SPI_DFF_16  = 1U                    // 16-bit data-frame
} SPI_dff_t;

// ====================== SPI clock-polarity enum ==================
typedef enum
{
    SPI_CPOL_LOW = 0U,                  // clock is idle on 0
    SPI_CPOL_HIGH = 1U,                 // clock is idle on 1
} SPI_cpol_t;

// ====================== SPI clock-phase enum =====================
typedef enum
{
    SPI_CPHA_FIRST_EDGE     = 0U,       // data is sampled on first-edge
    SPI_CPHA_SECOND_EDGE    = 1U,       // data is sampled on second-edge
} SPI_cpha_t;

// ====================== SPI ssm enum =============================
typedef enum
{
    SPI_SSM_DI = 0U,                    // software slave management disabled
    SPI_SSM_EN = 1U                     // software slave management enabled
} SPI_ssm_t;

// ====================== Configuration structure for SPI ==========
typedef struct
{
    SPI_device_mode_t       SPI_DeviceMode;     // Master/Slave
    SPI_bus_config_t        SPI_BusConfig;      // Half/Full-Duplex
    SPI_baud_prescaler_t    SPI_BaudPrescaler;  // Serial-Clock Speed
    SPI_dff_t               SPI_DFF;            // Data-Frame Format
    SPI_cpol_t              SPI_CPOL;           // Clock-Polarity
    SPI_cpha_t              SPI_CPHA;           // Clock-Phase
    SPI_ssm_t               SPI_SSM;            // Slave-Select Management

} SPI_config_t;

// ====================== Handle structure for SPI ======================
typedef struct
{
    SPI_reg_t *pSPIx;
    SPI_config_t SPI_Config;
} SPI_handle_t;

/*********************************************************************************
 * 							APIs supported by this driver
 * 					Check function definitions for more information
 *********************************************************************************/

status_t 	SPI_ConfigInit(SPI_config_t *pConfig);								    // Sets SPI_config_t struct to safe default values
status_t	SPI_Init(SPI_handle_t *pSPIHandle);								        // Configures a SPI pin according to SPI_config_t struct
status_t 	SPI_Reset(SPI_reg_t *pSPIx);										    // Resets a SPI peripheral to default register values

status_t	SPI_PeriClkCtl(SPI_reg_t *pSPIx, uint8_t enable);					    // Can enable/disable the clock for a given SPI base addr

status_t    SPI_Send(SPI_reg_t *pSPIx, uint8_t *pTXBuffer, uint32_t len);           // Send data via SPI
status_t    SPI_Receive(SPI_reg_t *pSPIx, uint8_t *pRXBuffer, uint32_t len);        // Receive data via SPI

status_t 	SPI_IRQEnable(uint8_t IRQNumber);										// Enables IRQ number in the NVIC
status_t 	SPI_IRQDisable(uint8_t IRQNumber);										// Disables IRQ number in the NVIC
status_t 	SPI_IRQPriority(uint8_t IRQNumber, uint8_t IRQPriority);				// Sets IRQ priority for IRQ number
status_t 	SPI_IRQHandling(SPI_handle_t *pHandle);									// Clears EXTI pending bit

#endif /* INC_STM32F407XX_SPI_DRIVER_H_ */

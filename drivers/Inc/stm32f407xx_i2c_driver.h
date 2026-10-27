/*
 * stm32f407xx_i2c_driver.h
 *
 *  Created on: Oct 27, 2026
 *      Author: ernad
 */

#ifndef INC_STM32F407XX_I2C_DRIVER_H_
#define INC_STM32F407XX_I2C_DRIVER_H_

#include "stm32f407xx.h"

// ====================== I2C application event macros =====================
typedef enum
{
    I2C_EVENT_PLACEHOLDER = 0    // TODO
} I2C_event_t;

// ====================== I2C SCL-speed enum =============================
typedef enum
{
    I2C_SCL_SPEED_SM = 100000U,             // 100KHz
    I2C_SCL_SPEED_FM = 400000U,             // 400KHz
} I2C_scl_speed_t;

// ====================== I2C ACK-control enum =============================
typedef enum
{
    I2C_ACK_DI = 0U,                        // Disable automatic ACKing
    I2C_ACK_EN = 1U,                        // Enable automatic ACKing
} I2C_ack_ctl_t;

// ====================== I2C duty cycle enum =============================
typedef enum
{
    I2C_FM_DUTY_2       = 0U,               // TODO
    I2C_FM_DUTY_16_9    = 1U                // TODO
} I2C_duty_cycle_t;

// ====================== Configuration structure for I2C ==================
typedef struct
{
    I2C_scl_speed_t     I2C_SCLSpeed;       // Clock-speed
    uint8_t             I2C_DeviceAddr;     // Own device slave address
    I2C_ack_ctl_t       I2C_ACKControl;     // En/Di automatic ACKing
    I2C_duty_cycle_t    I2C_FMDutyCycle;    // SCLK duty cycle for fast-mode
} I2C_config_t;

// ====================== Handle structure for I2C ======================
typedef struct
{
    I2C_reg_t       *pI2Cx;
    I2C_config_t    I2C_Cconfig;
} I2C_handle_t;

/*********************************************************************************
 * 							APIs supported by this driver
 * 					Check function definitions for more information
 *********************************************************************************/

status_t I2C_ConfigInit(I2C_config_t *pConfig);					    // Sets I2C_config_t struct to safe default values
status_t I2C_Init(I2C_handle_t *pI2CHandle);						// Configures an I2C pin according to I2C_config_t struct
status_t I2C_Reset(I2C_reg_t *pI2Cx);								// Resets a I2C peripheral to default register values

status_t I2C_PeriClkCtl(I2C_reg_t *pI2Cx, uint8_t enable);          // Enables/disables the clock for a given I2C base addr

status_t I2C_PeripheralControl(I2C_reg_t *pI2Cx, uint8_t enable);   // Enables or disables PE bit in I2C_CR1 register

uint8_t  I2C_GetFlag(I2C_reg_t *pI2Cx, uint32_t flagBit);           // Returns value of SR1/SR2 at flagBit position

void I2C_ApplicationEventCallback(I2C_handle_t *pHandle,
                                  I2C_event_t event);               // Weakly implemented, called by private driver interrupt handlers


#endif /* INC_STM32F407XX_I2C_DRIVER_H_ */

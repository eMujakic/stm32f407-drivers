/*
 * stm32f407xx_i2c_driver.h
 *
 *  Created on: Oct 27, 2026
 *      Author: ernad
 */

#ifndef INC_STM32F407XX_I2C_DRIVER_H_
#define INC_STM32F407XX_I2C_DRIVER_H_

#include "stm32f407xx.h"

#define I2C_TIMEOUT 100000U // Number of iterations to wait for I2C flag

#define I2C_TRISE_MAX_SM 1000U  // in nanoseconds
#define I2C_TRISE_MAX_FM 300U   // in nanoseconds

// ====================== I2C State Macros ======================
#define I2C_READY               0U
#define I2C_BUSY_IN_RX          1U
#define I2C_BUSY_IN_TX          2U
#define I2C_RECOVERY_REQUIRED   3U

// ====================== Repeated-Start Macros ======================
#define I2C_SR_EN   ENABLE
#define I2C_SR_DI   DISABLE

// ====================== I2C Mode enum =====================
typedef enum
{
    I2C_MODE_SLAVE  = 0U,
    I2C_MODE_MASTER = 1U
} I2C_mode_t;

// ====================== I2C flags enum =============================
typedef enum
{
    I2C_SB_FLAG = 0U,
    I2C_ADDR_FLAG = 1U,
    I2C_BTF_FLAG = 2U,
    I2C_ADD10_FLAG = 3U,
    I2C_STOPF_FLAG = 4U,
    I2C_RxNE_FLAG = 5U,
    I2C_TxE_FLAG = 6U,
    I2C_BERR_FLAG = 7U,
    I2C_ARLO_FLAG = 8U,
    I2C_AF_FLAG = 9U,
    I2C_OVR_FLAG = 10U,
    I2C_PECERR_FLAG = 11U,
    I2C_TIMEOUT_FLAG = 12U,
    I2C_SMBALERT_FLAG = 13U,
    I2C_MSL_FLAG = 14U,
    I2C_BUSY_FLAG = 15U,
    I2C_TRA_FLAG = 16U,
    I2C_GENCALL_FLAG = 17U,
    I2C_SMBDEFAULT_FLAG = 18U,
    I2C_SMBHOST_FLAG = 19U,
    I2C_DUALF_FLAG = 20U,
    I2C_PEC_FLAG = 21U,
} I2C_flag_t;

// ====================== I2C Read/Write enum =====================
typedef enum
{
    I2C_WRITE   = 0U,
    I2C_READ    = 1U
} I2C_rw_t;

// ====================== I2C application event enum =====================
typedef enum
{
    I2C_EVT_INVALID_STATE       = 0U,
    I2C_EVT_TX_DONE             = 1U,
    I2C_EVT_RX_DONE             = 2U,
    I2C_EVT_STOP                = 3U,
    I2C_EVT_BERR                = 4U,
    I2C_EVT_ARLO                = 5U,
    I2C_EVT_NACK                = 6U,
    I2C_EVT_SLAVE_ADDR_MATCH    = 7U,
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
    I2C_FM_DUTY_2       = 0U,               // Fast mode: T_low / T_high = 2    (low-time to high-time ratio)
    I2C_FM_DUTY_16_9    = 1U                // Fast mode: T_low / T_high = 16/9 (low-time to high-time ratio)
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
    I2C_config_t    I2C_Config;

    // Interrupt state
    uint8_t         *pBuffer;
    uint32_t        remaining;      // number of bytes remaining in buffer
    uint8_t         TxRxState;      // stores communication state
    uint8_t         targetAddr;     // address of target device to communicate with
    uint32_t        transferSize;   // Initial length of transfer
    uint8_t         Sr;             // stores repeated start value
    I2C_mode_t      mode;           // None/Master/Slave
} I2C_handle_t;

/*********************************************************************************
 * 							APIs supported by this driver
 * 					Check function definitions for more information
 *********************************************************************************/

status_t I2C_ConfigInit(I2C_config_t *pConfig);					    // Sets I2C_config_t struct to safe default values
status_t I2C_Init(I2C_handle_t *pI2CHandle);						// Configures an I2C pin according to I2C_config_t struct
status_t I2C_Reset(I2C_reg_t *pI2Cx);								// Resets a I2C peripheral to default register values

status_t I2C_MasterSendData(I2C_handle_t *pI2CHandle,               
    uint8_t *pTxBuffer, uint32_t len, uint8_t slaveAddr,
    uint8_t SR);                                                    // Sends data to slave as master.

status_t I2C_MasterReceiveData(I2C_handle_t *pI2CHandle,
    uint8_t *pRxBuffer, uint32_t len, uint8_t slaveAddr,
    uint8_t SR);                                                    // Receives data from slave as master.

status_t I2C_PeriClkCtl(I2C_reg_t *pI2Cx, uint8_t enable);          // Enables/disables the clock for a given I2C base addr

status_t I2C_PeripheralControl(I2C_reg_t *pI2Cx, uint8_t enable);   // Enables or disables PE bit in I2C_CR1 register

status_t I2C_SoftwareReset(I2C_reg_t *pI2Cx);                       // Performs a software reset on the I2C peripheral

uint8_t  I2C_GetFlag(I2C_reg_t *pI2Cx, I2C_flag_t flag);            // Returns value of SR1/SR2 at flag position
status_t I2C_WaitForFlag(I2C_reg_t *pI2Cx, I2C_flag_t flag,         // Spins until flag==val or timeout occurs
    uint8_t val);


// Interrupt APIs

status_t I2C_MasterSendDataIT(I2C_handle_t *pI2CHandle,               
    uint8_t *pTxBuffer, uint32_t len, uint8_t slaveAddr,
    uint8_t SR);                                                    // Sends data to slave as master (non-blocking)

status_t I2C_MasterReceiveDataIT(I2C_handle_t *pI2CHandle,
    uint8_t *pRxBuffer, uint32_t len, uint8_t slaveAddr,
    uint8_t SR);                                                    // Receives data from slave as master (non-blocking)

status_t I2C_IRQHandling_EV(I2C_handle_t *pI2CHandle);              // Dispatcher for event interrupts.
status_t I2C_IRQHandling_ERR(I2C_handle_t *pI2CHandle);             // Dispatcher for error interrupts.

void I2C_ApplicationEventCallback(I2C_handle_t *pHandle,
                                  I2C_event_t event);               // Weakly implemented, called by private driver interrupt handlers


#endif /* INC_STM32F407XX_I2C_DRIVER_H_ */

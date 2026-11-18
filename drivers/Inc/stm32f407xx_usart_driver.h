/*
 * stm32f407xx_usart_driver.h
 *
 *  Created on: Nov 13, 2026
 *      Author: ernad
 */

#ifndef INC_STM32F407XX_USART_DRIVER_H_
#define INC_STM32F407XX_USART_DRIVER_H_

#include "stm32f407xx.h"

#define USART_TIMEOUT 100000U // USART flag polling budget

// ====================== USART State Macros ======================
#define USART_READY             0U
#define USART_BUSY              1U  
#define USART_RECOVERY_REQUIRED 2U

// ====================== USART baud-rate macros =============================
#define USART_BAUD_9600     9600U
#define USART_BAUD_19200    19200U
#define USART_BAUD_28800    28800U
#define USART_BAUD_38400    38400U
#define USART_BAUD_57600    57600U
#define USART_BAUD_76800    76800U
#define USART_BAUD_115200   115200U
#define USART_BAUD_230400   230400U
#define USART_BAUD_460800   460800U
#define USART_BAUD_576000   576000U
#define USART_BAUD_921600   921600U

// ====================== USART event enum =============================
typedef enum
{
    USART_EVT_INVALID_STATE     = 0U,
    USART_EVT_TX_DONE           = 1U,
    USART_EVT_RX_DONE           = 2U,
    USART_EVT_CTS               = 3U,
    USART_EVT_OVR               = 4U,
    USART_EVT_NF                = 5U,
    USART_EVT_FE                = 6U,
    USART_EVT_PE                = 7U,
    USART_EVT_IDLE              = 8U,
} USART_event_t;

// ====================== USART Flag enum =============================
typedef enum
{
    USART_FLAG_PE   = 0U, // parity-error
    USART_FLAG_FE   = 1U, // framing error
    USART_FLAG_NF   = 2U, // noise-detected
    USART_FLAG_ORE  = 3U, // overrun error
    USART_FLAG_IDLE = 4U, // idle-line detected
    USART_FLAG_RXNE = 5U, // receive-buffer not-empty
    USART_FLAG_TC   = 6U, // transmission complete
    USART_FLAG_TXE  = 7U, // transmit-buffer empty
    USART_FLAG_LBD  = 8U, // LIN break detection
    USART_FLAG_CTS  = 9U  // CTS flag
} USART_flag_t;

// ====================== USART Word-Length enum =============================
// note: the USART word length includes the parity bit if parity is enabled
// note: the parity bit is stored in the MSB
typedef enum
{
    USART_WORD_LEN_8 = 0U,
    USART_WORD_LEN_9 = 1U
} USART_word_len_t;

// ====================== USART Parity Control enum =============================
typedef enum
{
    USART_PARITY_DISABLED   = 0U,
    USART_PARITY_EVEN       = 1U,
    USART_PARITY_ODD        = 2U
} USART_parity_t;

// ====================== USART Stop-Bits enum =============================
typedef enum
{
    USART_STOP_BITS_1   = 0U,
    USART_STOP_BITS_0_5 = 1U,   // 0.5 stop bits (not supported by UART4 & UART5)
    USART_STOP_BITS_2   = 2U,
    USART_STOP_BITS_1_5 = 3U,   // 1.5 stop bits (not supported by UART4 & UART5)
} USART_stop_bits_t;

// ====================== USART Mode enum =============================
typedef enum
{
    USART_MODE_TX_ONLY  = 0U,
    USART_MODE_RX_ONLY  = 1U,
    USART_MODE_TX_RX    = 2U
} USART_mode_t;

// ====================== USART oversampling enum =============================
typedef enum
{
    USART_OVERSAMPLING_16   = 0U,
    USART_OVERSAMPLING_8    = 1U
} USART_oversampling_t;

// ====================== USART hw flow-control enum =============================
// note: hardware flow-control not supported by UART4/UART5
typedef enum
{
    USART_HW_FLOW_CTL_NONE      = 0U,
    USART_HW_FLOW_CTL_RTS       = 1U,
    USART_HW_FLOW_CTL_CTS       = 2U,
    USART_HW_FLOW_CTL_RTS_CTS   = 3U
} USART_hw_flow_ctl_t;

// ====================== Configuration structure for USART ==================
typedef struct
{
    USART_mode_t            USART_Mode;
    uint32_t                USART_Baud;
    USART_stop_bits_t       USART_NumStopBits;
    USART_word_len_t        USART_WordLen;
    USART_parity_t          USART_ParityCtl;
    USART_hw_flow_ctl_t     USART_HWFlowCtl;
    USART_oversampling_t    USART_Oversampling;
} USART_config_t;

// ====================== Handle structure for USART ======================
typedef struct
{
    USART_reg_t     *pUSARTx;
    USART_config_t  USART_Config;

    // ============= Interrupt State =============
    uint8_t                 *pRxBuffer;
    uint8_t                 *pTxBuffer;

    uint32_t                RxLen;
    uint32_t                TxLen;

    volatile uint8_t        RxState;
    volatile uint8_t        TxState;
} USART_handle_t;

/*********************************************************************************
 * 							APIs supported by this driver
 * 					Check function definitions for more information
 *********************************************************************************/

// ===================== Config APIs =====================
status_t USART_ConfigInit(USART_config_t *pConfig);				    // Sets USART_config_t struct to safe default values
status_t USART_Init(USART_handle_t *pUSARTHandle);				    // Configures an USART peripheral according to USART_config_t
status_t USART_Reset(USART_reg_t *pUSARTx);					        // Resets a USART peripheral to default register values

// ===================== Transfer APIs =====================
status_t USART_SendData(USART_handle_t *pUSARTHandle,
    uint8_t *pTxBuffer, uint32_t len);                              // Send data using USART
status_t USART_ReceiveData(USART_handle_t *pUSARTHandle,
    uint8_t *pRxBuffer, uint32_t len);                              // Receive data using USART

// ===================== Control APIs =====================
status_t USART_PeripheralControl(USART_reg_t *pUSARTx,
    uint8_t enable);                                                // Enables or disables USART peripheral
status_t USART_PeriClkCtl(USART_reg_t *pUSARTx, uint8_t enable);    // Enables/disables the clock for a given USART base addr

// ===================== Flag APIs =====================
uint8_t  USART_GetFlag(USART_reg_t *pUSARTx, USART_flag_t flag);    // Returns value of a flag for a USART peripheral
status_t USART_WaitForFlag(USART_reg_t *pUSARTx,
    USART_flag_t flag, uint8_t val);                                // Spins until flag==val or timeout occurs

// ===================== Interrupt APIs =====================
void USART_IRQHandling(USART_handle_t *pUSARTHandle);               // Dispatcher for USART interrupts

status_t USART_SendDataIT(USART_handle_t *pUSARTHandle,
    uint8_t *pTxBuffer, uint32_t len);                              // Interrupt-driven USART Send
status_t USART_ReceiveDataIT(USART_handle_t *pUSARTHandle,
    uint8_t *pRxBuffer, uint32_t len);                              // Interrupt-driven USART Receive

status_t USART_CTSNotificationCtl(USART_handle_t *pHandle,
    uint8_t EnOrDi);                                                // Enables/Disables CTS event notifications

void USART_ApplicationEventCallback(USART_handle_t *pHandle,
                                  USART_event_t event);               // Weakly implemented, called by private driver interrupt handlers

#endif /* INC_STM32F407XX_USART_DRIVER_H_ */
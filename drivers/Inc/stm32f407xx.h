/*
 * stm32f407xx.h
 *
 *  Created on: Jun 10, 2026
 *      Author: ernad
 */

#ifndef INC_STM32F407XX_H_
#define INC_STM32F407XX_H_

//  ======================= base addresses of FLASH and SRAM memories =======================
#define FLASH_BASE_ADDR 			0x08000000UL				// the base addr of the flash memory
#define SRAM1_BASE_ADDR 			0x20000000UL				// 112 KBs
#define SRAM2_BASE_ADDR 			0x20001C00UL				// 16 KBs
#define ROM										0x1FFF0000UL				// base addr of system memory
#define SRAM 									SRAM1_BASE_ADDR

//  ======================= base addresses of AHBx and APBx bus domains =======================
#define APB1_BASE_ADDR				0x40000000UL				// low-speed peripheral bus 1
#define APB2_BASE_ADDR				0x40010000UL				// low-speed peripheral bus 2
#define AHB1_BASE_ADDR				0x40020000UL				// high-speed peripheral bus 1
#define AHB2_BASE_ADDR				0x50000000 UL			// high-speed peripheral bus 2

//  ======================= base addresses of AHB1 peripherals =======================
#define GPIOA_BASE_ADDR			0x40020000UL			// GPIO port A register set base addr
#define GPIOB_BASE_ADDR			0x40020400UL			// GPIO port B register set base addr
#define GPIOC_BASE_ADDR			0x40020800UL			// GPIO port C register set base addr
#define GPIOD_BASE_ADDR			0x40020C00UL			// GPIO port D register set base addr
#define GPIOE_BASE_ADDR			0x40021000UL			// GPIO port E register set base addr
#define GPIOF_BASE_ADDR			0x40021400UL			// GPIO port F register set base addr
#define GPIOG_BASE_ADDR			0x40021800UL			// GPIO port G register set base addr
#define GPIOH_BASE_ADDR			0x40021C00UL			// GPIO port H register set base addr
#define GPIOI_BASE_ADDR			0x40022000UL			// GPIO port I register set base addr
#define GPIOJ_BASE_ADDR			0x40022400UL			// GPIO port J register set base addr
#define GPIOK_BASE_ADDR			0x40022800UL			// GPIO port K register set base addr

#define DMA1_BASE_ADDR			0x40026000UL			// base addr for DMA1 register set
#define DMA2_BASE_ADDR			0x40026400UL			// base addr for DMA2 register set

// ======================= base addresses of APB1 peripherals =======================
#define SPI2_BASE_ADDR				0x40003800UL			// base addr of SPI2 register set
#define SPI3_BASE_ADDR				0x40003C00UL			// base addr of SPI2 register set

#define USART2_BASE_ADDR		0x40004400UL			// base addr of USART2 register set
#define USART3_BASE_ADDR		0x40004800UL			// base addr of USART3 register set

#define UART4_BASE_ADDR			0x40004C00UL			// base addr of UART4 register set
#define UART5_BASE_ADDR			0x40005000UL			// base addr of UART5 register set

#define I2C1_BASE_ADDR				0x40005400UL			// base addr of I2C1 register set
#define I2C2_BASE_ADDR				0x40005800UL			// base addr of I2C2 register set
#define I2C3_BASE_ADDR				0x40005C00UL			// base addr of I2C3 register set

#define UART7_BASE_ADDR			0x40007800UL			// base addr of UART7 register set
#define UART8_BASE_ADDR			0x40007C00UL			// base addr of UART8 register set

// ======================= base addresses of APB2 peripherals =======================
#define USART1_BASE_ADDR		0x40011000UL			// base addr of USART1 register set
#define USART6_BASE_ADDR		0x40011400UL			// base addr of USART6 register set

#define SPI1_BASE_ADDR				0x40013000UL			// base addr of SPI1 register set
#define SPI4_BASE_ADDR				0x40013400UL			// base addr of SPI4 register set

#define SYSCFG_BASE_ADDR			0x40013800UL			// base addr of SYSCFG register set

#define EXTI_BASE_ADDR				0x40013C00UL			// base addr of EXTI register set

#define SPI5_BASE_ADDR				0x40015000UL			// base addr of SPI5 register set
#define SPI6_BASE_ADDR				0x40015400UL			// base addr of SPI6 register set

// ======================= GPIO register struct =======================
typedef struct
{
	uint32_t MODER;						// determines whether the GPIO line is input (00), output (01), alternate function (10), or analog (11) mode
	uint32_t OTYPER;						// determines whether GPIO line is push-pull (0), or open-drain (1)
	uint32_t OSPEEDR;						// determines output speed of GPIO line, low-speed (00), med. speed (01), high-speed (10), very high speed (11)
	uint32_t PUPDR;							// sets pull-up/down resistors for GPIO line, no PU/PD (00), PU (01), PD (10), RESERVED (11)
	uint32_t IDR;								// the input-data register for a GPIO port (read-only)
	uint32_t ODR;								// the output-data register for a GPIO port
	uint32_t BSRR;								// GPIO port set/reset register
	uint32_t LCKR;								// locks the config.of the port bits
	uint32_t AFR[2];						// alternate function LOW & HIGH register
} GPIO_REG_T;

// ======================= SPI register struct =======================
typedef struct
{
	uint32_t CR1;								// SPI control reg. 1
	uint32_t CR2;								// SPI control reg. 2
	uint32_t SR;									// SPI status register
	uint32_t DR;									// SPI data register
	uint32_t CRCPR;							// CRC polynomial register
	uint32_t RXCRCR;						// RX CRC register
	uint32_t TXCRCR;						// TX CRC register
	uint32_t I2SCFGR;						// I2S config. register
	uint32_t I2SPR;							// I2S prescaler register
} SPI_REG_T;


#endif /* INC_STM32F407XX_H_ */









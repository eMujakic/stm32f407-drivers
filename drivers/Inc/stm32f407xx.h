/*
 * stm32f407xx.h
 *
 *  Created on: Jun 10, 2026
 *      Author: ernad
 */

#ifndef INC_STM32F407XX_H_
#define INC_STM32F407XX_H_

#include<stdint.h>

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

#define RCC_BASE_ADDR				0x40023800UL			// RCC engine register set base addr

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

//#define UART7_BASE_ADDR			0x40007800UL			// base addr of UART7 register set
//#define UART8_BASE_ADDR			0x40007C00UL			// base addr of UART8 register set

// ======================= base addresses of APB2 peripherals =======================
#define USART1_BASE_ADDR		0x40011000UL			// base addr of USART1 register set
#define USART6_BASE_ADDR		0x40011400UL			// base addr of USART6 register set

#define SPI1_BASE_ADDR				0x40013000UL			// base addr of SPI1 register set
#define SPI4_BASE_ADDR				0x40013400UL			// base addr of SPI4 register set

#define SYSCFG_BASE_ADDR			0x40013800UL			// base addr of SYSCFG register set

#define EXTI_BASE_ADDR				0x40013C00UL			// base addr of EXTI register set

#define SPI5_BASE_ADDR				0x40015000UL			// base addr of SPI5 register set
#define SPI6_BASE_ADDR				0x40015400UL			// base addr of SPI6 register set






// ======================= RCC register struct =======================
typedef struct
{
	volatile uint32_t CR;										// clock control register
	volatile uint32_t PLLCFGR;							// PLL config. register
	volatile uint32_t CFGR;									// clock config. register
	volatile uint32_t CIR;									// clock interrupt register
	volatile uint32_t AHB1RSTR;						// AHB1 peripheral reset register
	volatile uint32_t AHB2RSTR;						// AHB2 peripheral reset register
	volatile uint32_t AHB3RSTR;						// AHB3 peripheral reset register
	volatile uint32_t __RESERVED_0;				// RESERVED
	volatile uint32_t APB1RSTR;						// APB1 peripheral reset register
	volatile uint32_t APB2RSTR;						// APB2 peripheral reset register
	volatile uint32_t __RESERVED_1[2];			// RESERVED
	volatile uint32_t AHB1ENR;							// AHB1 peripheral clock enable register
	volatile uint32_t AHB2ENR;							// AHB2 peripheral clock enable register
	volatile uint32_t AHB3ENR;							// AHB3 peripheral clock enable register
	volatile uint32_t __RESERVED_2;				// RESERVED
	volatile uint32_t APB1ENR;							// APB1 peripheral clock enable register
	volatile uint32_t APB2ENR;							// APB2 peripheral clock enable register
	volatile uint32_t __RESERVED_3[2];			// RESERVED
	volatile uint32_t AHB1LPENR;					// AHB1 peripheral clock enable in low-power mode register
	volatile uint32_t AHB2LPENR;					// AHB2 peripheral clock enable in low-power mode register
	volatile uint32_t AHB3LPENR;					// AHB3 peripheral clock enable in low-power mode register
	volatile uint32_t __RESERVED_4;				// RESERVED
	volatile uint32_t APB1LPENR;						// APB1 peripheral clock enable in low power mode register
	volatile uint32_t APB2LPENR;						// APB2 peripheral clock enable in low power mode register
	volatile uint32_t __RESERVED_5[2];			// RESERVED
	volatile uint32_t BDCR;								// backup domain control register
	volatile uint32_t CSR;									// clock control & status register
	volatile uint32_t __RESERVED_6[2];			// RESERVED
	volatile uint32_t SSCGR;								// spread spectrum clock generation register
	volatile uint32_t PLLI2SCFGR;					// PLLI2S configuration register
} RCC_REG_T;

// ======================= GPIO register struct =======================
typedef struct
{
	volatile uint32_t MODER;						// determines whether the GPIO line is input (00), output (01), alternate function (10), or analog (11) mode
	volatile uint32_t OTYPER;						// determines whether GPIO line is push-pull (0), or open-drain (1)
	volatile uint32_t OSPEEDR;						// determines output speed of GPIO line, low-speed (00), med. speed (01), high-speed (10), very high speed (11)
	volatile uint32_t PUPDR;							// sets pull-up/down resistors for GPIO line, no PU/PD (00), PU (01), PD (10), RESERVED (11)
	volatile uint32_t IDR;								// the input-data register for a GPIO port (read-only)
	volatile uint32_t ODR;								// the output-data register for a GPIO port
	volatile uint32_t BSRR;								// GPIO port set/reset register
	volatile uint32_t LCKR;								// locks the config.of the port bits
	volatile uint32_t AFR[2];							// alternate function LOW [0] & HIGH register
} GPIO_REG_T;

// ======================= SPI register struct =======================
typedef struct
{
	volatile uint32_t CR1;								// SPI control reg. 1
	volatile uint32_t CR2;								// SPI control reg. 2
	volatile uint32_t SR;									// SPI status register
	volatile uint32_t DR;									// SPI data register
	volatile uint32_t CRCPR;							// CRC polynomial register
	volatile uint32_t RXCRCR;						// RX CRC register
	volatile uint32_t TXCRCR;						// TX CRC register
	volatile uint32_t I2SCFGR;						// I2S config. register
	volatile uint32_t I2SPR;							// I2S prescaler register
} SPI_REG_T;







// =======================  type-casted macros =======================
#define GPIOA					(GPIO_REG_T*)		GPIOA_BASE_ADDR
#define GPIOB					(GPIO_REG_T*)		GPIOB_BASE_ADDR
#define GPIOC					(GPIO_REG_T*)		GPIOC_BASE_ADDR
#define GPIOD					(GPIO_REG_T*)		GPIOD_BASE_ADDR
#define GPIOE					(GPIO_REG_T*)		GPIOE_BASE_ADDR
#define GPIOF					(GPIO_REG_T*)		GPIOF_BASE_ADDR
#define GPIOG					(GPIO_REG_T*)		GPIOG_BASE_ADDR
#define GPIOH					(GPIO_REG_T*)		GPIOH_BASE_ADDR
#define GPIOI					(GPIO_REG_T*)		GPIOI_BASE_ADDR
#define GPIOJ					(GPIO_REG_T*)		GPIOJ_BASE_ADDR
#define GPIOK					(GPIO_REG_T*)		GPIOK_BASE_ADDR

#define RCC						(RCC_REG_T*)		 	RCC_BASE_ADDR





// ======================= GPIO clock enable/disable macros =======================
#define GPIOA_PCLK_EN()				(  RCC->AHB1ENR |= ( 1 << 0  )  )
#define GPIOB_PCLK_EN()				(  RCC->AHB1ENR |= ( 1 << 1  )  )
#define GPIOC_PCLK_EN()				(  RCC->AHB1ENR |= ( 1 << 2  )  )
#define GPIOD_PCLK_EN()				(  RCC->AHB1ENR |= ( 1 << 3  )  )
#define GPIOE_PCLK_EN()				(  RCC->AHB1ENR |= ( 1 << 4  )  )
#define GPIOF_PCLK_EN()				(  RCC->AHB1ENR |= ( 1 << 5  )  )
#define GPIOG_PCLK_EN()				(  RCC->AHB1ENR |= ( 1 << 6  )  )
#define GPIOH_PCLK_EN()				(  RCC->AHB1ENR |= ( 1 << 7  )  )
#define GPIOI_PCLK_EN()				(  RCC->AHB1ENR |= ( 1 << 8  )  )

#define GPIOA_PCLK_DI() 				(  RCC->AHB1ENR &= ~( 1 << 0  )  )
#define GPIOB_PCLK_DI()				(  RCC->AHB1ENR &= ~( 1 << 1  )  )
#define GPIOC_PCLK_DI()				(  RCC->AHB1ENR &= ~( 1 << 2  )  )
#define GPIOD_PCLK_DI()				(  RCC->AHB1ENR &= ~( 1 << 3  )  )
#define GPIOE_PCLK_DI()				(  RCC->AHB1ENR &= ~( 1 << 4  )  )
#define GPIOF_PCLK_DI()				(  RCC->AHB1ENR &= ~( 1 << 5  )  )
#define GPIOG_PCLK_DI()				(  RCC->AHB1ENR &= ~( 1 << 6  )  )
#define GPIOH_PCLK_DI()				(  RCC->AHB1ENR &= ~( 1 << 7  )  )
#define GPIOI_PCLK_DI()				(  RCC->AHB1ENR &= ~( 1 << 8  )  )

// ======================= 	I2C clock enable/disable macros =======================
#define I2C1_PCLK_EN()					( RCC->APB1ENR |= ( 1 << 21 ) )
#define I2C2_PCLK_EN()					( RCC->APB1ENR |= ( 1 << 22 ) )
#define I2C3_PCLK_EN()					( RCC->APB1ENR |= ( 1 << 23 ) )

#define I2C1_PCLK_DI()					( RCC->APB1ENR &= ~( 1 << 21 ) )
#define I2C2_PCLK_DI()					( RCC->APB1ENR &= ~( 1 << 22 ) )
#define I2C3_PCLK_DI()					( RCC->APB1ENR &= ~( 1 << 23 ) )

// ======================= 	SPI clock enable/disable macros =======================
#define SPI1_PCLK_EN()					( RCC->APB2ENR |= ( 1 << 12 ) )
#define SPI2_PCLK_EN()					( RCC->APB1ENR |= ( 1 << 14 ) )
#define SPI3_PCLK_EN()					( RCC->APB1ENR |= ( 1 << 15 ) )

#define SPI1_PCLK_DI()					( RCC->APB2ENR &= ~( 1 << 12 ) )
#define SPI2_PCLK_DI()					( RCC->APB1ENR &= ~( 1 << 14 ) )
#define SPI3_PCLK_DI()					( RCC->APB1ENR &= ~( 1 << 15 ) )

// ======================= 	SYSCFG clock enable/disable macros =======================
#define SYSCFG_PCLK_EN()				( RCC->APB2ENR |= ( 1 << 14 ) )

#define SYSCFG_PCLK_DI()				( RCC->APB2ENR &= ~( 1 << 14 ) )

// ======================= 	USART clock enable/disable macros =======================
#define USART1_PCLK_EN()			( RCC->APB2ENR |= ( 1 << 4 ) )
#define USART2_PCLK_EN()			( RCC->APB1ENR |= ( 1 << 17 ) )
#define USART3_PCLK_EN()			( RCC->APB1ENR |= ( 1 << 18 ) )
#define UART4_PCLK_EN()				( RCC->APB1ENR |= ( 1 << 19 ) )
#define UART5_PCLK_EN()				( RCC->APB1ENR |= ( 1 << 20 ) )
#define USART6_PCLK_EN()			( RCC->APB2ENR |= ( 1 << 5 ) )

#define USART1_PCLK_DI()			( RCC->APB2ENR &= ~( 1 << 4 ) )
#define USART2_PCLK_DI()			( RCC->APB1ENR &= ~( 1 << 17 ) )
#define USART3_PCLK_DI()			( RCC->APB1ENR &= ~( 1 << 18 ) )
#define UART4_PCLK_DI()			( RCC->APB1ENR &= ~( 1 << 19 ) )
#define UART5_PCLK_DI()			( RCC->APB1ENR &= ~( 1 << 20 ) )
#define USART6_PCLK_DI()			( RCC->APB2ENR &= ~( 1 << 5 ) )

#endif /* INC_STM32F407XX_H_ */









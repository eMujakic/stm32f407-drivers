/*
 * @file		005SPI_TX_test.c
 * @author		ernad
 * @date 		Aug 12, 2026
 * @brief		Basic 2-Line SPI send data test
 *
 * @details
 * This application tests basic SPI-send functionality by configuring GPIO pins as SPI2
 * pins which can then be read using a logic analyzer.
 */

#include <stdint.h>
#include "stm32f407xx.h"

#define SPI2_AF         GPIO_AF5
#define SPI2_PORT       GPIOB

#define SPI2_MOSI_PIN   GPIO_PIN_15   
#define SPI2_SCLK_PIN   GPIO_PIN_13
#define SPI2_NSS_PIN    GPIO_PIN_12

static void gpio_init()
{
    GPIO_handle_t gpio_spi2;
    GPIO_ConfigInit(&gpio_spi2.GPIO_PinConfig);

    // MOSI
    gpio_spi2.pGPIOx = SPI2_PORT;
	gpio_spi2.GPIO_PinConfig.pinNumber = SPI2_MOSI_PIN;
	gpio_spi2.GPIO_PinConfig.mode = GPIO_MODE_ALTFN;
	gpio_spi2.GPIO_PinConfig.outType = GPIO_OTYPE_PP;
	gpio_spi2.GPIO_PinConfig.puPdCtl  = GPIO_NO_PUPD;
	gpio_spi2.GPIO_PinConfig.speed  = GPIO_SPEED_HIGH;
    gpio_spi2.GPIO_PinConfig.altFuncMode = SPI2_AF;
    GPIO_Init(&gpio_spi2);

    // SCLK
    gpio_spi2.GPIO_PinConfig.pinNumber = SPI2_SCLK_PIN;
	GPIO_Init(&gpio_spi2);
}

static void spi2_init(SPI_handle_t *spi2_handle)
{
    spi2_handle->pSPIx = SPI2;
    SPI_ConfigInit(&spi2_handle->SPI_Config);

    spi2_handle->SPI_Config.SPI_DeviceMode      = SPI_MODE_MASTER;
    spi2_handle->SPI_Config.SPI_BaudPrescaler   = SPI_BAUD_DIV4;
    spi2_handle->SPI_Config.SPI_BusConfig       = SPI_BUS_HALF_DUPLEX_TX;
    spi2_handle->SPI_Config.SPI_DFF 		    = SPI_DFF_8;
    spi2_handle->SPI_Config.SPI_CPOL            = SPI_CPOL_LOW;
    spi2_handle->SPI_Config.SPI_CPHA            = SPI_CPHA_FIRST_EDGE;
    spi2_handle->SPI_Config.SPI_SSM             = SPI_SSM_EN;   // automatically enables SSI

    SPI_Init(spi2_handle);
    SPI_PeripheralControl(SPI2, ENABLE);
}

int main(void)
{    
    gpio_init();

    SPI_handle_t spi2_handle;
    spi2_init(&spi2_handle);

    char msg[] = "Hello World!";
    uint32_t len = sizeof(msg) - 1U;

    while(1)    // loop indefinitely
        SPI_Send(spi2_handle.pSPIx, (uint8_t*)msg, len);
    
    return 0;
}

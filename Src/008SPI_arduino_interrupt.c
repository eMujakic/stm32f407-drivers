/*
 * 008SPI_arduino_interrupt.c
 *
 *  Created on: Aug 19, 2026
 *      Author: ernad
 */

#include <stdint.h>
#include "stm32f407xx.h"
#include <string.h>
#include <stdio.h>

#define SPI2_AF         GPIO_AF5        // SPI2 alternate-function mode
#define SPI2_PORT       GPIOB           // SPI2 GPIO port

#define SPI2_MOSI_PIN   GPIO_PIN_15   
#define SPI2_MISO_PIN   GPIO_PIN_14
#define SPI2_SCLK_PIN   GPIO_PIN_13
#define SPI2_NSS_PIN    GPIO_PIN_12

// GPIO port/pin of external interrupt line
#define INT_PORT		GPIOD
#define INT_PIN			GPIO_PIN_9

// Initializes GPIO pins for SPI2 peripheral
static void gpio_init()
{
    GPIO_handle_t gpio_spi2;
    GPIO_ConfigInit(&gpio_spi2.GPIO_PinConfig);

    gpio_spi2.pGPIOx = SPI2_PORT;
    gpio_spi2.GPIO_PinConfig.mode           = GPIO_MODE_ALTFN;  // alternate-functionality mode
	gpio_spi2.GPIO_PinConfig.outType        = GPIO_OTYPE_PP;    // push-pull
	gpio_spi2.GPIO_PinConfig.puPdCtl        = GPIO_NO_PUPD;
	gpio_spi2.GPIO_PinConfig.speed          = GPIO_SPEED_HIGH;
    gpio_spi2.GPIO_PinConfig.altFuncMode    = SPI2_AF;          // set AF mode to SPI2

    // MOSI
	gpio_spi2.GPIO_PinConfig.pinNumber      = SPI2_MOSI_PIN;
    GPIO_Init(&gpio_spi2);

    // MISO
    gpio_spi2.GPIO_PinConfig.pinNumber      = SPI2_MISO_PIN;
    GPIO_Init(&gpio_spi2);

    // SCLK
    gpio_spi2.GPIO_PinConfig.pinNumber      = SPI2_SCLK_PIN;
	GPIO_Init(&gpio_spi2);

    // NSS
    gpio_spi2.GPIO_PinConfig.pinNumber      = SPI2_NSS_PIN;
    GPIO_Init(&gpio_spi2);
}

// Initializes SPI2 peripheral
static void spi2_init(SPI_handle_t *spi2_handle)
{
    spi2_handle->pSPIx = SPI2;
    SPI_ConfigInit(&spi2_handle->SPI_Config);

    spi2_handle->SPI_Config.SPI_DeviceMode      = SPI_MODE_MASTER;
    spi2_handle->SPI_Config.SPI_BaudPrescaler   = SPI_BAUD_DIV16;      	// 1MHz
    spi2_handle->SPI_Config.SPI_BusConfig       = SPI_BUS_FULL_DUPLEX;
    spi2_handle->SPI_Config.SPI_DFF 		    = SPI_DFF_8;
    spi2_handle->SPI_Config.SPI_CPOL            = SPI_CPOL_LOW;
    spi2_handle->SPI_Config.SPI_CPHA            = SPI_CPHA_FIRST_EDGE;
    spi2_handle->SPI_Config.SPI_SSM             = SPI_SSM_DI;           // no software slave-management

    SPI_Init(spi2_handle);
}

// configures GPIO pin as interrupt input pin
static void gpio_interrupt_pin_init()
{
	GPIO_handle_t intHandle;
	GPIO_ConfigInit(&intHandle.GPIO_PinConfig);

	intHandle.pGPIOx 							= INT_PORT;
	intHandle.GPIO_PinConfig.pinNumber 			= INT_PIN;
	intHandle.GPIO_PinConfig.mode           	= GPIO_MODE_IT_FT;	// interrupt, falling-trigger
	intHandle.GPIO_PinConfig.puPdCtl        	= GPIO_NO_PUPD;     // pulled-high by Arduino
	intHandle.GPIO_PinConfig.speed          	= GPIO_SPEED_LOW;

    GPIO_Init(&intHandle);

    NVIC_SetPriority( IRQ_NUM_EXTI5_9, IRQ_PRI15 );     // Set IRQ priority to max
    NVIC_EnableIRQ( IRQ_NUM_EXTI5_9 );                  // Enable interrupts on the IRQ number
}

// global variables
#define MAX_LEN 500
char rcv_buf[MAX_LEN];

SPI_handle_t spi2_handle;

volatile char readByte;
volatile uint8_t rcvStop = 0;

// this flag will be set in the GPIO interrupt handler
volatile uint8_t dataAvailable = 0;

int main(void)
{
    // initialize all GPIO pins
	gpio_init();
	gpio_interrupt_pin_init();

	spi2_init(&spi2_handle);

	// enables nss output, follows value of SPE (spi peripheral-enable)
	SPI_SSOEConfig(SPI2, ENABLE);
	
	// enables IRQ number for SPI2 in processor NVIC
	NVIC_EnableIRQ(IRQ_NUM_SPI2);

    printf("Starting Loop\n");
	while(1)
	{
        rcvStop = 0;

        // spin until data available (mimics busy work)
        while(!dataAvailable);
        printf("Data Available!\n");

        NVIC_DisableIRQ(IRQ_NUM_EXTI5_9);       // disable interrupts until done handling current one

        SPI_PeripheralControl(SPI2, ENABLE);    // also pulls NSS low

        while( !rcvStop )
        {
            // keep trying to set-up RX interrupt state in handle until successful
            while( SPI_ReceiveIT(&spi2_handle, &readByte, 1) == STATUS_BUSY );
            
            // wait until done reading
            while (spi2_handle.RxState == SPI_BUSY_RX);

            if (readByte == '\0')
                rcvStop = 1;        // break
        }

        // spin until not busy
        while ( SPI_GetFlag(SPI2, SPI_SR_BSY_BIT) );

        SPI_PeripheralControl(SPI2, DISABLE);   // pulls NSS back high

        printf("Received Data: %s\n", rcv_buf); // prints data to SWV (configured in syscalls.c)

        dataAvailable = 0;
        NVIC_EnableIRQ(IRQ_NUM_EXTI5_9);        // re-enable interrupts on GPIO pin
	}

	return 0;
}

// runs when a byte is received over SPI
void SPI2_IRQHandler(void)
{
    SPI_IRQHandling(&spi2_handle);
}

// sets null-terminator after RX_DONE
void SPI_ApplicationEventCallback(SPI_handle_t *pSPIx, SPI_event_t event)
{

    static uint8_t i = 0;

    if ( event == SPI_EVENT_RX_DONE )
    {
        rcv_buf[i++] = readByte;
        if ( readByte == '\0' || (i == MAX_LEN) )
        {
            rcvStop = 1;
            rcv_buf[i-1] = '\0';
            i = 0;
        }
    }
}

/* Slave data available interrupt handler */
void EXTI9_5_IRQHandler(void)
{
    dataAvailable = 1;
	GPIO_IRQHandling(INT_PIN);
}

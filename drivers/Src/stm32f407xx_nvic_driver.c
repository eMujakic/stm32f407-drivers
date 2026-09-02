/*
 * stm32f407xx_nvic_driver.c
 *
 *  Created on: Aug 22, 2026
 *      Author: ernad
 */

#include "stm32f407xx_nvic_driver.h"


/**************************************************************************
 * @fn		NVIC_EnableIRQ
 *
 * @brief	Enables a peripheral interrupt in the NVIC.
 *
 * @details
 * Sets the corresponding bit in the NVIC Interrupt Set-Enable Register
 * for the passed IRQ number. Writing 1 to an ISER bit enables the IRQ,
 * while writing 0 has no effect. So writing the register directly
 * (rather than a read-modify-write) works fine.
 *
 * @param[in] IRQNumber     IRQ number identifying interrupt source.
 *
 * @return	(STATUS_OK) if command was successful.
 *			(STATUS_INVALID_PARAM) if IRQNumber is out of range.
 ***************************************************************************/
status_t NVIC_EnableIRQ(IRQ_num_t IRQNumber)
{
    if ( IRQNumber > IRQ_NUM_MAX )
    {
        return STATUS_INVALID_PARAM;
    }
        
    NVIC_ISER[IRQNumber / 32U] = (1U << (IRQNumber % 32U)); // uses write instead of read-modify-write
    return STATUS_OK;
}

/**************************************************************************
 * @fn		NVIC_DisableIRQ
 *
 * @brief	Disables a peripheral interrupt in the NVIC.
 *
 * @details
 * Sets the corresponding bit in the NVIC Interrupt Clear-Enable Register
 * for the passed IRQ number. Writing 1 to an ICER bit disables the IRQ,
 * while writing 0 has no effect. So writing the register directly
 * (rather than a read-modify-write) works fine.
 *
 * @param[in] IRQNumber     IRQ number identifying interrupt source.
 *
 * @return	(STATUS_OK) if command was successful.
 *			(STATUS_INVALID_PARAM) if IRQNumber is out of range.
 *
 ***************************************************************************/
status_t NVIC_DisableIRQ(IRQ_num_t IRQNumber)
{
    if ( IRQNumber > IRQ_NUM_MAX )
    {
        return STATUS_INVALID_PARAM;
    }

    NVIC_ICER[IRQNumber / 32U] = (1U << (IRQNumber % 32U));		// uses write instead of read-modify-write
    return STATUS_OK;
}
 
/**************************************************************************
 * @fn		NVIC_SetPriority
 *
 * @brief	Sets the priority level for an IRQ number in the NVIC.
 *
 * @details
 * Updates the priority field for the specified IRQ in the NVIC priority
 * register. Each IRQ priority field is 8-bits wide, but only the upper
 * NO_PR_BITS_IMPLEMENTED (4) bits are used by the processor. The
 * requested priority level, therefore, is written into the upper-nibble
 * of the corresponding register byte.
 *
 * @param[in] IRQNumber     IRQ number identifying interrupt source.
 * @param[in] priority      Desired priority level for the IRQ.
 *
 * @return	(STATUS_OK) if command was successful.
 *			(STATUS_INVALID_PARAM) if IRQNumber/priority is out of range.
 ***************************************************************************/
status_t NVIC_SetPriority(IRQ_num_t IRQNumber, IRQ_priority_t priority)
{
    // validate args
    if ( IRQNumber > IRQ_NUM_MAX )
    {
        return STATUS_INVALID_PARAM;
    }
    if ( priority > IRQ_PRI15 )
    {
        return STATUS_INVALID_PARAM;
    }
        
    uint32_t reg_idx 	= IRQNumber / 4U;
	uint32_t offset 	= ((IRQNumber % 4U) * 8U);

    // cast priority from enum to uint32_t
    uint32_t priority_value = (uint32_t) priority;

	// Mask priority to implemented priority bits
	// (e.g., 0x0FU if NO_PR_BITS_IMPLEMENTED == 4)
	priority_value &= ((1U << NO_PR_BITS_IMPLEMENTED) - 1U);

	// clear current priority bits
	NVIC_IPR[reg_idx] &= ~(0xFFU << offset);
	// set new priority bits, bits must be shifted to upper nibble
	NVIC_IPR[reg_idx] |= (priority_value << (offset + (8U - NO_PR_BITS_IMPLEMENTED)));	

	return STATUS_OK;
}

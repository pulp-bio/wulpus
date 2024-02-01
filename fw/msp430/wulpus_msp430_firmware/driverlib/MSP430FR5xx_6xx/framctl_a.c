//*****************************************************************************
//
// framctl_a.c - Driver for the framctl_a Module.
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup framctl_a_api framctl_a
//! @{
//
//*****************************************************************************

#include "inc/hw_memmap.h"

#ifdef __MSP430_HAS_FRCTL_A__
#include "framctl_a.h"

#include <assert.h>

void FRAMCtl_A_write8(uint8_t *dataPtr,
    uint8_t *framPtr,
    uint16_t numberOfBytes
    )
{
    while (numberOfBytes > 0)
    {
        //Write to Fram
        *framPtr++ = *dataPtr++;
        numberOfBytes--;
    }
}

void FRAMCtl_A_write16(uint16_t *dataPtr,uint16_t *framPtr,
		uint16_t numberOfWords)
{
    while (numberOfWords > 0)
    {
        //Write to Fram
        *framPtr++ = *dataPtr++;
        numberOfWords--;
    }
}

void FRAMCtl_A_write32(uint32_t *dataPtr,uint32_t *framPtr,
		uint16_t count)
{
    while (count > 0)
    {
        //Write to Fram
        *framPtr++ = *dataPtr++;
        count--;
    }
}

void FRAMCtl_A_fillMemory32 (uint32_t value,
    uint32_t *framPtr,
    uint16_t count
    )
{
    while (count> 0)
    {
        //Write to Fram
        *framPtr++ = value;
        count--;
    }
}

void FRAMCtl_A_enableInterrupt (uint8_t interruptMask)
{
    uint8_t temp = HWREG8(FRCTL_A_BASE + OFS_FRCTL0_L);

    // Clear lock in FRAM control registers
    HWREG16(FRCTL_A_BASE + OFS_FRCTL0) = FRCTLPW | temp;

    // Enable user selected interrupt sources
    HWREG16(FRCTL_A_BASE + OFS_GCCTL0) |= interruptMask;
}

uint8_t FRAMCtl_A_getInterruptStatus(uint16_t interruptFlagMask)
{
    return ( HWREG16(FRCTL_A_BASE + OFS_GCCTL1) & interruptFlagMask );
}

void FRAMCtl_A_disableInterrupt(uint16_t interruptMask)
{
    uint8_t temp = HWREG8(FRCTL_A_BASE + OFS_FRCTL0_L);

    //Clear lock in FRAM control registers
    HWREG16(FRCTL_A_BASE + OFS_FRCTL0) = FRCTLPW | temp;

    HWREG16(FRCTL_A_BASE + OFS_GCCTL0) &= ~(interruptMask);
}

void FRAMCtl_A_clearInterrupt(uint16_t interruptFlagMask)
{
    uint8_t temp = HWREG8(FRCTL_A_BASE + OFS_FRCTL0_L);

    //Clear lock in FRAM control registers
    HWREG16(FRCTL_A_BASE + OFS_FRCTL0) = FRCTLPW | temp;

    HWREG16(FRCTL_A_BASE + OFS_GCCTL1) &= ~interruptFlagMask;
}

void FRAMCtl_A_configureWaitStateControl(uint8_t waitState )
{
    uint8_t temp = HWREG8(FRCTL_A_BASE + OFS_FRCTL0_L);
    
    temp &= ~NWAITS_15;
    temp |= waitState;
    HWREG16(FRCTL_A_BASE + OFS_FRCTL0) = FRCTLPW | temp;
}

void FRAMCtl_A_delayPowerUpFromLPM(uint8_t delayStatus)
{
    uint8_t temp = HWREG8(FRCTL_A_BASE + OFS_FRCTL0_L);

    // Clear lock in FRAM control registers
    HWREG16(FRCTL_A_BASE + OFS_FRCTL0) = FRCTLPW | temp;

    HWREG8(FRCTL_A_BASE + OFS_GCCTL0_L) &= ~FRPWR;
    HWREG8(FRCTL_A_BASE + OFS_GCCTL0_L) |= delayStatus;
}

void FRAMCtl_A_enableWriteProtection(void)
{
    uint8_t temp = HWREG8(FRCTL_A_BASE + OFS_FRCTL0_L);
    temp |= WPROT;
    HWREG16(FRCTL_A_BASE + OFS_FRCTL0) = FRCTLPW | temp;
}

void FRAMCtl_A_disableWriteProtection(void)
{
    uint8_t temp = HWREG8(FRCTL_A_BASE + OFS_FRCTL0_L);
    temp &= ~WPROT;
    HWREG16(FRCTL_A_BASE + OFS_FRCTL0) = FRCTLPW | temp;
}

#endif
//*****************************************************************************
//
//! Close the doxygen group for framctl_a_api
//! @}
//
//*****************************************************************************

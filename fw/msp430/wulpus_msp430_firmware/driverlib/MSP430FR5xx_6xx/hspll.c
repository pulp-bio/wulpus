//*****************************************************************************
//
// hspll.c - Driver for the HSPLL Module.
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup hspll_api hspll
//! @{
//
//*****************************************************************************

#include "inc/hw_memmap.h"

#ifdef __MSP430_HAS_HSPLL__
#include "hspll.h"

#include <assert.h>

void HSPLL_init(uint16_t baseAddress, HSPLL_initParam *param)
{
    assert (param->multiplier > 15);
    assert (param->multiplier < 40);
    
    HWREG16(baseAddress + OFS_HSPLLCTL) =
        (param->multiplier << 10)
        | param->frequency 
        | param->lockStatus;
}

void HSPLL_xtalInit(uint16_t baseAddress, HSPLL_xtalInitParam *param)
{    
    HWREG16(baseAddress + OFS_HSPLLUSSXTLCTL) =
        param->oscillatorType
        | param->xtlOutput
        | param->oscillatorEnable;
}

uint16_t HSPLL_getInterruptStatus(uint16_t baseAddress)
{
    return (HWREG16(baseAddress + OFS_HSPLLRIS) & PLLUNLOCK);
}

uint16_t HSPLL_getInterruptMaskStatus(uint16_t baseAddress)
{
    return (HWREG16(baseAddress + OFS_HSPLLIMSC) & PLLUNLOCK);
}

void HSPLL_enableInterrupt(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_HSPLLIMSC) |= PLLUNLOCK;
}

void HSPLL_disableInterrupt(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_HSPLLIMSC) &= ~PLLUNLOCK;
}

void HSPLL_clearInterrupt(uint16_t baseAddress)        
{
    HWREG16(baseAddress + OFS_HSPLLICR) |= PLLUNLOCK;
}

void HSPLL_setInterrupt(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_HSPLLISR) |= PLLUNLOCK;
}

uint16_t HSPLL_getOscillatorStatus(uint16_t baseAddress)
{
    return (HWREG16(baseAddress + OFS_HSPLLUSSXTLCTL) & OSCSTATE);
}

uint16_t HSPLL_isLocked(uint16_t baseAddress)
{
    return (HWREG16(baseAddress + OFS_HSPLLCTL) & PLL_LOCK);
}


#endif
//*****************************************************************************
//
//! Close the doxygen group for hspll_api
//! @}
//
//*****************************************************************************


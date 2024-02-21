//*****************************************************************************
//
// ram.c - Driver for the ram Module.
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup ram_api ram
//! @{
//
//*****************************************************************************

#include "inc/hw_memmap.h"

#ifdef __MSP430_HAS_RC_FRAM__
#include "ram.h"

#include <assert.h>

void RAM_setSectorOff(uint8_t sector, uint8_t mode)
{
    uint8_t sectorPos = sector<<1;
    uint8_t val = HWREG8(RAM_BASE + OFS_RCCTL0_L) & ~(0x3 << sectorPos);

    HWREG16(RAM_BASE + OFS_RCCTL0) = (RCKEY | val | (mode << sectorPos));
}

uint8_t RAM_getSectorState (uint8_t sector)
{
    uint8_t sectorPos = sector<<1;
    return (HWREG8(RAM_BASE + OFS_RCCTL0_L) & (0x3<<sectorPos)) >> sectorPos;
}

#endif
//*****************************************************************************
//
//! Close the doxygen group for ram_api
//! @}
//
//*****************************************************************************

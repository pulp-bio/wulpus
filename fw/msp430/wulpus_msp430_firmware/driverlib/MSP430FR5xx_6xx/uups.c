//*****************************************************************************
//
// uups.c - Driver for the UUPS Module.
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup uups_api uups
//! @{
//
//*****************************************************************************

#include "inc/hw_memmap.h"

#ifdef __MSP430_HAS_UUPS__
#include "uups.h"

#include <assert.h>

void UUPS_clearInterrupt(uint16_t baseAddress, uint8_t mask)
{
    HWREG16(baseAddress + OFS_UUPSICR) |= mask;
}

void UUPS_enableInterrupt(uint16_t baseAddress, uint8_t mask)
{
    HWREG16(baseAddress + OFS_UUPSIMSC) |= mask;
}

void UUPS_disableInterrupt(uint16_t baseAddress, uint8_t mask)
{
    HWREG16(baseAddress + OFS_UUPSIMSC) &= ~mask;
}

uint8_t UUPS_getInterruptStatus(uint16_t baseAddress, uint16_t mask)
{
    return HWREG16(baseAddress + OFS_UUPSRIS) & mask;
}

void UUPS_stopMeasurement(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_UUPSCTL) |= USSSTOP;
}

void UUPS_turnOffPower(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_UUPSCTL) |= USSPWRDN;
}

void UUPS_turnOnPower(uint16_t baseAddress, uint16_t triggerSource)
{
    HWREG16(baseAddress + OFS_UUPSCTL) |= (triggerSource | USSPWRUP);
}

void UUPS_enableASQ(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_UUPSCTL) |= ASQEN;
}

uint8_t UUPS_getPowerModeStatus(uint16_t baseAddress)
{
    return HWREG16(baseAddress + OFS_UUPSCTL) & UPSTATE;
}

uint8_t UUPS_isBusy(uint16_t baseAddress)
{
    return HWREG16(baseAddress + OFS_UUPSCTL) & USS_BUSY;
}

uint8_t UUPS_isLDOReady(uint16_t baseAddress)
{
    return HWREG16(baseAddress + OFS_UUPSCTL) & LDORDY;
}

void UUPS_setLowPowerBiasHoldOffDelay(uint16_t baseAddress, uint16_t holdOffDelay)
{
    HWREG16(baseAddress + OFS_UUPSCTL) &= ~LBHDEL;
    HWREG16(baseAddress + OFS_UUPSCTL) |= holdOffDelay;
}

void UUPS_holdUSSInResetState(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_UUPSCTL) |= USSSWRST;
}

void UUPS_releaseUSSFromResetState(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_UUPSCTL) &= ~USSSWRST;
}

#endif
//*****************************************************************************
//
//! Close the doxygen group for uups_api
//! @}
//
//*****************************************************************************


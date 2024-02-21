//*****************************************************************************
//
// crc.c - Driver for the crc Module.
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup crc_api crc
//! @{
//
//*****************************************************************************

#include "inc/hw_memmap.h"

#ifdef __MSP430_HAS_CRC__
#include "crc.h"

#include <assert.h>

void CRC_setSeed (uint16_t baseAddress,
    uint16_t seed)
{
    HWREG16(baseAddress + OFS_CRCINIRES) = seed;
}

void CRC_set16BitData (uint16_t baseAddress,
    uint16_t dataIn)
{
    HWREG16(baseAddress + OFS_CRCDI) = dataIn;
}

void CRC_set8BitData (uint16_t baseAddress,
    uint8_t dataIn)
{
    HWREG8(baseAddress + OFS_CRCDI_L) = dataIn;
}

void CRC_set16BitDataReversed (uint16_t baseAddress,
    uint16_t dataIn)
{
    HWREG16(baseAddress + OFS_CRCDIRB) = dataIn;
}

void CRC_set8BitDataReversed (uint16_t baseAddress,
    uint8_t dataIn)
{
    HWREG8(baseAddress + OFS_CRCDIRB_L) = dataIn;
}

uint16_t CRC_getData (uint16_t baseAddress)
{
    return ( HWREG16(baseAddress + OFS_CRCDI) );
}

uint16_t CRC_getResult (uint16_t baseAddress)
{
    return ( HWREG16(baseAddress + OFS_CRCINIRES) );
}

uint16_t CRC_getResultBitsReversed (uint16_t baseAddress)
{
    return ( HWREG16(baseAddress + OFS_CRCRESR) );
}

#endif
//*****************************************************************************
//
//! Close the doxygen group for crc_api
//! @}
//
//*****************************************************************************

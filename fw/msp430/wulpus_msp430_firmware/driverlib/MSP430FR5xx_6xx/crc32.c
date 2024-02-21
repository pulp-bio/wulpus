//*****************************************************************************
//
// crc32.c - Driver for the crc32 Module.
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup crc32_api crc32
//! @{
//
//*****************************************************************************

#include "inc/hw_memmap.h"

#ifdef __MSP430_HAS_CRC32__
#include "crc32.h"

#include <assert.h>

void CRC32_setSeed(uint32_t seed, uint8_t crcMode)
{
    if (CRC16_MODE == crcMode) {
        HWREG16(CRC32_BASE + OFS_CRC16INIRESW0) = seed;
    }
    else
    {
        HWREG16(CRC32_BASE + OFS_CRC32INIRESW1) = ((seed & 0xFFFF0000)
                >> 16);
        HWREG16(CRC32_BASE + OFS_CRC32INIRESW0) = (seed & 0xFFFF);
    }
}

void CRC32_set8BitData(uint8_t dataIn, uint8_t crcMode)
{
    if (CRC16_MODE == crcMode) {
        HWREG8(CRC32_BASE + OFS_CRC16DIW0_L) = dataIn;
    }
    else {
        HWREG8(CRC32_BASE + OFS_CRC32DIW0_L) = dataIn;
    }
}

void CRC32_set16BitData(uint16_t dataIn, uint8_t crcMode)
{
    if (CRC16_MODE == crcMode) {
        HWREG16(CRC32_BASE + OFS_CRC16DIW0) = dataIn;
    }
    else {
        HWREG16(CRC32_BASE + OFS_CRC32DIW0) = dataIn;
    }

}

void CRC32_set32BitData(uint32_t dataIn)
{

    HWREG16(CRC32_BASE + OFS_CRC32DIW0) = dataIn & 0xFFFF;
    HWREG16(CRC32_BASE + OFS_CRC32DIW1) = (uint16_t) ((dataIn & 0xFFFF0000)
            >> 16);
}

void CRC32_set8BitDataReversed(uint8_t dataIn, uint8_t crcMode)
{

    if (CRC16_MODE == crcMode) {
        HWREG8(CRC32_BASE + OFS_CRC16DIRBW0_L) = dataIn;
    } else {
        HWREG8(CRC32_BASE + OFS_CRC32DIRBW1_L) = dataIn;
    }
}

void CRC32_set16BitDataReversed(uint16_t dataIn, uint8_t crcMode)
{

    if (CRC16_MODE == crcMode) {
        HWREG16(CRC32_BASE + OFS_CRC16DIRBW0) = dataIn;
    } else {
        HWREG16(CRC32_BASE + OFS_CRC32DIRBW1) = dataIn;
    }
}

void CRC32_set32BitDataReversed(uint32_t dataIn)
{
    HWREG16(CRC32_BASE + OFS_CRC32DIRBW1) = dataIn & 0xFFFF;
    HWREG16(CRC32_BASE + OFS_CRC32DIRBW0) = (uint16_t) ((dataIn & 0xFFFF0000)
            >> 16);
}

uint32_t CRC32_getResult(uint8_t crcMode)
{
    if (CRC16_MODE == crcMode) {
        return (HWREG16(CRC32_BASE + OFS_CRC16INIRESW0) );
    }
    else
    {
        uint32_t result = 0;
        result = HWREG16(CRC32_BASE + OFS_CRC32INIRESW1);
        result = (result << 16);
        result |= HWREG16(CRC32_BASE + OFS_CRC32INIRESW0);
        return (result);
    }
}

uint32_t CRC32_getResultReversed(uint8_t crcMode)
{
    if (CRC16_MODE == crcMode) {
        return (HWREG16(CRC32_BASE + OFS_CRC16RESRW0) );
    }
    else
    {
        uint32_t result = 0;
        result = HWREG16(CRC32_BASE + OFS_CRC32RESRW0);
        result = (result << 16);
        result |= HWREG16(CRC32_BASE + OFS_CRC32RESRW1);
        return (result);
    }
}

#endif
//*****************************************************************************
//
//! Close the doxygen group for crc32_api
//! @}
//
//*****************************************************************************

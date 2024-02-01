//*****************************************************************************
//
// mpy32.c - Driver for the mpy32 Module.
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup mpy32_api mpy32
//! @{
//
//*****************************************************************************

#include "inc/hw_memmap.h"

#ifdef __MSP430_HAS_MPY32__
#include "mpy32.h"

#include <assert.h>

void MPY32_setWriteDelay (uint16_t writeDelaySelect)
{
    HWREG16(MPY32_BASE + OFS_MPY32CTL0) &= ~(MPYDLY32 + MPYDLYWRTEN);
    HWREG16(MPY32_BASE + OFS_MPY32CTL0) |= writeDelaySelect;
}

void MPY32_enableSaturationMode (void)
{
    HWREG8(MPY32_BASE + OFS_MPY32CTL0_L) |= MPYSAT;
}

void MPY32_disableSaturationMode (void)
{
    HWREG8(MPY32_BASE + OFS_MPY32CTL0_L) &= ~(MPYSAT);
}

uint8_t MPY32_getSaturationMode (void)
{
    return (HWREG8(MPY32_BASE + OFS_MPY32CTL0_L) &(MPYSAT));
}

void MPY32_enableFractionalMode (void)
{
    HWREG8(MPY32_BASE + OFS_MPY32CTL0_L) |= MPYFRAC;
}

void MPY32_disableFractionalMode (void)
{
    HWREG8(MPY32_BASE + OFS_MPY32CTL0_L) &= ~(MPYFRAC);
}

uint8_t MPY32_getFractionalMode (void)
{
    return (HWREG8(MPY32_BASE + OFS_MPY32CTL0_L) &(MPYFRAC));
}

void MPY32_setOperandOne8Bit (uint8_t multiplicationType,
    uint8_t operand)
{
    HWREG8(MPY32_BASE + OFS_MPY + multiplicationType) = operand;
}

void MPY32_setOperandOne16Bit (uint8_t multiplicationType,
    uint16_t operand)
{
    HWREG16(MPY32_BASE + OFS_MPY + multiplicationType) = operand;
}

void MPY32_setOperandOne24Bit (uint8_t multiplicationType,
    uint32_t operand)
{
    multiplicationType <<= 1;
    HWREG16(MPY32_BASE + OFS_MPY32L + multiplicationType) = operand;
    HWREG8(MPY32_BASE + OFS_MPY32H + multiplicationType) = (operand >> 16);
}

void MPY32_setOperandOne32Bit (uint8_t multiplicationType,
    uint32_t operand)
{
    multiplicationType <<= 1;
    HWREG16(MPY32_BASE + OFS_MPY32L + multiplicationType) = operand;
    HWREG16(MPY32_BASE + OFS_MPY32H + multiplicationType) = (operand >> 16);
}

void MPY32_setOperandTwo8Bit (uint8_t operand)
{
    HWREG8(MPY32_BASE + OFS_OP2) = operand;
}

void MPY32_setOperandTwo16Bit (uint16_t operand)
{
    HWREG16(MPY32_BASE + OFS_OP2) = operand;
}

void MPY32_setOperandTwo24Bit (uint32_t operand)
{
    HWREG16(MPY32_BASE + OFS_OP2L) = operand;
    HWREG8(MPY32_BASE + OFS_OP2H) = (operand >> 16);
}

void MPY32_setOperandTwo32Bit (uint32_t operand)
{
    HWREG16(MPY32_BASE + OFS_OP2L) = operand;
    HWREG16(MPY32_BASE + OFS_OP2H) = (operand >> 16);
}

uint64_t MPY32_getResult (void)
{
    uint64_t result;

    result = HWREG16(MPY32_BASE + OFS_RES0);
    result += ((uint64_t)HWREG16(MPY32_BASE + OFS_RES1) << 16);
    result += ((uint64_t)HWREG16(MPY32_BASE + OFS_RES2) << 32);
    result += ((uint64_t)HWREG16(MPY32_BASE + OFS_RES3) << 48);
    return ( result );
}

uint16_t MPY32_getSumExtension (void)
{
    return ( HWREG16(MPY32_BASE + OFS_SUMEXT) );
}

uint16_t MPY32_getCarryBitValue (void)
{
    return ( HWREG16(MPY32_BASE + OFS_MPY32CTL0) | MPYC);
}
void MPY32_clearCarryBitValue (void)
{
    HWREG16(MPY32_BASE + OFS_MPY32CTL0) &= ~MPYC;
}
void MPY32_preloadResult (uint64_t result)
{
    HWREG16(MPY32_BASE + OFS_RES0) = (result & 0xFFFF);
    HWREG16(MPY32_BASE + OFS_RES1) = ((result >> 16) & 0xFFFF);
    HWREG16(MPY32_BASE + OFS_RES2) = ((result >> 32) & 0xFFFF);
    HWREG16(MPY32_BASE + OFS_RES3) = ((result >> 48) & 0xFFFF);
}

#endif
//*****************************************************************************
//
//! Close the doxygen group for mpy32_api
//! @}
//
//*****************************************************************************

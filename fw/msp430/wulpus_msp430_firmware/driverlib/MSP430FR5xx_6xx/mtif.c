//*****************************************************************************
//
// mtif.c - Driver for the MTIF Module.
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup mtif_api mtif
//! @{
//
//*****************************************************************************

#include "inc/hw_memmap.h"

#ifdef __MSP430_HAS_MTIF__
#include "mtif.h"

#include <assert.h>

void MTIF_enablePulseGen(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_MTIFPGCNF) = (PGPW | PGEN);
}

void MTIF_disablePulseGen(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_MTIFPGCNF) = PGPW;
}

void MTIF_clearPulseGenCounter(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_MTIFPGCNF) = (PGPW | PGCLR);
}

void MTIF_setPulseGenPulseGridFreq(uint16_t baseAddress, uint8_t freq)
{
    uint8_t state = HWREG8(baseAddress + OFS_MTIFPGCNF_L);
    state &= ~PGFS;
    state |= freq;
    HWREG16(baseAddress + OFS_MTIFPGCNF) = (PGPW | state);
}

void MTIF_setPulseGenCountNum(uint16_t baseAddress, uint8_t num)
{
    HWREG16(baseAddress + OFS_MTIFPGKVAL) = (PGPW | num);
}

void MTIF_clearPulseKCountUpdateRequest(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_MTIFPGCTL) = (PGPW | PKUR);
}

void MTIF_clearPulseGridFreqUpdateRequest(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_MTIFPGCTL) = (PGPW | PGUR);
}

void MTIF_setPulseKCountUpdateAck(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_MTIFPGSR) |= PKUA;
}

uint16_t MTIF_isPulseKCountUpdated(uint16_t baseAddress)
{
    return (HWREG16(baseAddress + OFS_MTIFPGSR) & PKUA);
}

void MTIF_setPulseGridFreqUpdateAck(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_MTIFPGSR) |= PGUA;
}

uint16_t MTIF_isPulseGridFreqUpdated(uint16_t baseAddress)
{
    return (HWREG16(baseAddress + OFS_MTIFPGSR) & PGUA);
}

void MTIF_enablePulseCounter(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_MTIFPCCNF) = (PCEN | PCPW);
}

void MTIF_disablePulseCounter(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_MTIFPCCNF) = PCPW;
}

void MTIF_clearPulseCounter(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_MTIFPCCNF) = (PCPW | PCCLR);
}

uint16_t MTIF_getPulseCount(uint16_t baseAddress)
{
    return HWREG16(baseAddress + OFS_MTIFPCR);
}

void MTIF_setPulseCounterReadRequest(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_MTIFPCCTL) |= PCRR;
}

void MTIF_setPulseCounterReadAck(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_MTIFPCSR) |= PCRA;
}

uint16_t MTIF_isPulseCounterReadReady(uint16_t baseAddress)
{
    return (HWREG16(baseAddress + OFS_MTIFPCSR) & PCRA);
}

uint16_t MTIF_getPulseCounterOverflow(uint16_t baseAddress)
{
    return (HWREG16(baseAddress + OFS_MTIFPCSR) & PCOFL);
}

void MTIF_ackPulseCounterOverflow(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_MTIFPCSR) |= PCOFL;
}

void MTIF_enableTestPortOutput(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_MTIFTPCTL) = (TPPW |
        HWREG8(baseAddress + OFS_MTIFTPCTL_L) | TPOE);
}

void MTIF_disableTestPortOutput(uint16_t baseAddress)
{
    uint8_t state = HWREG8(baseAddress + OFS_MTIFTPCTL_L);
    HWREG16(baseAddress + OFS_MTIFTPCTL) = (TPPW | (state & ~TPOE));
}

void MTIF_enableTestPortInput(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_MTIFTPCTL) = (TPPW |
        HWREG8(baseAddress + OFS_MTIFTPCTL_L) | TPIE);
}

void MTIF_disableTestPortInput(uint16_t baseAddress)
{
    uint8_t state = HWREG8(baseAddress + OFS_MTIFTPCTL_L);
    HWREG16(baseAddress + OFS_MTIFTPCTL) = (TPPW | (state & ~TPIE));
}

void MTIF_setPulseGeneratorAsPulseCounterInput(uint16_t baseAddress)
{
    uint8_t state = HWREG8(baseAddress + OFS_MTIFTPCTL_L);
    HWREG16(baseAddress + OFS_MTIFTPCTL) = (TPPW | (state & ~TPISEL));
}

void MTIF_setTestPortInputTerminalAsPulseCounterInput(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_MTIFTPCTL) = (TPPW |
        HWREG8(baseAddress + OFS_MTIFTPCTL_L) | TPISEL);
}

void MTIF_enableTestPortTerminalActivationBySW(uint16_t baseAddress)
{
    uint8_t state = HWREG8(baseAddress + OFS_MTIFTPCTL_L);
    HWREG16(baseAddress + OFS_MTIFTPCTL) = (TPPW | (state & ~ACTIVATE));
}

void MTIF_enableTestPortTerminalActivationBySWAndHW(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_MTIFTPCTL) = (TPPW |
        HWREG8(baseAddress + OFS_MTIFTPCTL_L) | ACTIVATE);
}

#endif
//*****************************************************************************
//
//! Close the doxygen group for mtif_api
//! @}
//
//*****************************************************************************


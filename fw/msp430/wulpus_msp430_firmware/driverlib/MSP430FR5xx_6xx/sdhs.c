//*****************************************************************************
//
// sdhs.c - Driver for the SDHS Module.
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup sdhs_api sdhs
//! @{
//
//*****************************************************************************

#include "inc/hw_memmap.h"

#ifdef __MSP430_HAS_SDHS__
#include "sdhs.h"

#include <assert.h>

void SDHS_init(uint16_t baseAddress, SDHS_initParam *param)
{
   //Unlock Registers
   HWREG16(baseAddress + OFS_SDHSCTL3) &= ~TRIGEN;
   HWREG16(baseAddress + OFS_SDHSCTL5) &= ~SDHS_LOCK;

    //Set SDHS Control 0
    HWREG16(baseAddress + OFS_SDHSCTL0) =
        param->triggerSourceSelect
        | param->msbShift 
        | param->outputBitResolution
        | param->dataFormat
        | param->dataAlignment
        | param->interruptDelayGeneration
        | param->autoSampleStart;
    
    //Set SDHS Control 1
    HWREG16(baseAddress + OFS_SDHSCTL1) =  param->oversamplingRate;
    
    //Set SDHS Control 2
    HWREG16(baseAddress + OFS_SDHSCTL2) = 
        param->dataTransferController
        | param->windowComparator
        | param->sampleSizeCounting;
}

void SDHS_setWindowComp(uint16_t baseAddress, uint16_t highThreshold,
    uint16_t lowThreshold)
{
    uint16_t saveCTL3 = HWREG16(baseAddress + OFS_SDHSCTL3);
    uint16_t saveCTL5 = HWREG16(baseAddress + OFS_SDHSCTL5);
    
    //Unlock Registers
    HWREG16(baseAddress + OFS_SDHSCTL3) &= ~TRIGEN;
    HWREG16(baseAddress + OFS_SDHSCTL5) &= ~SDHS_LOCK;
   
    HWREG16(baseAddress + OFS_SDHSWINHITH) = highThreshold;
    HWREG16(baseAddress + OFS_SDHSWINLOTH) = lowThreshold;
    
    //Restore CTL3 and CTL5
    HWREG16(baseAddress + OFS_SDHSCTL3) = saveCTL3;
    HWREG16(baseAddress + OFS_SDHSCTL5) = saveCTL5;
}

void SDHS_setTotalSampleSize(uint16_t baseAddress, uint16_t sampleSize)
{
    uint16_t saveCTL3 = HWREG16(baseAddress + OFS_SDHSCTL3);
    uint16_t saveCTL5 = HWREG16(baseAddress + OFS_SDHSCTL5);
    
    //Unlock Registers
    HWREG16(baseAddress + OFS_SDHSCTL3) &= ~TRIGEN;
    HWREG16(baseAddress + OFS_SDHSCTL5) &= ~SDHS_LOCK;
   
    HWREG16(baseAddress + OFS_SDHSCTL2) = sampleSize - 1;
}

void SDHS_enableTrigger(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_SDHSCTL3) |= TRIGEN;
}

void SDHS_disableTrigger(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_SDHSCTL3) &= ~TRIGEN;
}

void SDHS_enable(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_SDHSCTL4) |= SDHSON;
}

void SDHS_disable(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_SDHSCTL4) &= ~SDHSON;
}

uint16_t SDHS_getInterruptStatus(uint16_t baseAddress, uint16_t interruptMask)
{
    return (HWREG16(baseAddress + OFS_SDHSRIS) & interruptMask);
}

uint16_t SDHS_getInterruptMaskStatus(uint16_t baseAddress, uint16_t interruptMask)
{
    return (HWREG16(baseAddress + OFS_SDHSIMSC) & interruptMask);
}

void SDHS_enableInterrupt(uint16_t baseAddress, uint16_t interruptMask)
{
    HWREG16(baseAddress + OFS_SDHSIMSC) |= interruptMask;
}
void SDHS_disableInterrupt(uint16_t baseAddress, uint16_t interruptMask)
{
    HWREG16(baseAddress + OFS_SDHSIMSC) &= ~interruptMask;
}

void SDHS_clearInterrupt(uint16_t baseAddress, uint16_t interruptMask)
{
    HWREG16(baseAddress + OFS_SDHSICR) |= interruptMask;
}

void SDHS_setInterrupt(uint16_t baseAddress, uint16_t interruptMask)
{
    HWREG16(baseAddress + OFS_SDHSISR) |= interruptMask;
}

void SDHS_setPGAGain(uint16_t baseAddress, uint16_t gain)
{
    assert (gain < 0x40);
    HWREG16(baseAddress + OFS_SDHSCTL6) = gain;
}

void SDHS_setModularOptimization(uint16_t baseAddress, uint16_t optimization)
{
    assert (optimization < 0x20);
    HWREG16(baseAddress + OFS_SDHSCTL7) = optimization;
}
uint16_t SDHS_getRegisterLockStatus(uint16_t baseAddress)
{
    return (HWREG16(baseAddress + OFS_SDHSCTL5) & SDHS_LOCK);
}

void SDHS_startConversion(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_SDHSCTL5) |= SSTART;
}

void SDHS_endConversion(uint16_t baseAddress)
{
    HWREG16(baseAddress + OFS_SDHSCTL5) &= ~SSTART;
}

uint16_t SDHS_getResults(uint16_t baseAddress)
{
    return (HWREG16(baseAddress + OFS_SDHSDT));
}

void SDHS_setDTCDestinationAddress(uint16_t baseAddress, uint16_t address)
{
    HWREG16(baseAddress + OFS_SDHSDTCDA) = address;
}
#endif
//*****************************************************************************
//
//! Close the doxygen group for sdhs_api
//! @}
//
//*****************************************************************************


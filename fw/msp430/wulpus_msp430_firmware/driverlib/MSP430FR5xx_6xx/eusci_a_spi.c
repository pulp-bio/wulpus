//*****************************************************************************
//
// eusci_a_spi.c - Driver for the eusci_a_spi Module.
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup eusci_a_spi_api eusci_a_spi
//! @{
//
//*****************************************************************************

#include "inc/hw_memmap.h"

#ifdef __MSP430_HAS_EUSCI_Ax__
#include "eusci_a_spi.h"

#include <assert.h>

void EUSCI_A_SPI_initMaster (uint16_t baseAddress,
    EUSCI_A_SPI_initMasterParam *param)
{
    //Disable the USCI Module
    HWREG16(baseAddress + OFS_UCAxCTLW0) |= UCSWRST;

    //Reset OFS_UCAxCTLW0 values
    HWREG16(baseAddress + OFS_UCAxCTLW0) &= ~(UCCKPH + UCCKPL + UC7BIT + UCMSB +
        UCMST + UCMODE_3 + UCSYNC);

    //Reset OFS_UCAxCTLW0 values
    HWREG16(baseAddress + OFS_UCAxCTLW0) &= ~(UCSSEL_3);

    //Select Clock
    HWREG16(baseAddress + OFS_UCAxCTLW0) |= (uint16_t)param->selectClockSource;

    HWREG16(baseAddress + OFS_UCAxBRW) =
        (uint16_t)(param->clockSourceFrequency / param->desiredSpiClock);

    /*
     * Configure as SPI master mode.
     * Clock phase select, polarity, msb
     * UCMST = Master mode
     * UCSYNC = Synchronous mode
     * UCMODE_0 = 3-pin SPI
     */
    HWREG16(baseAddress + OFS_UCAxCTLW0) |= (
        param->msbFirst +
        param->clockPhase +
        param->clockPolarity +
        UCMST +
        UCSYNC +
        param->spiMode
        );
    //No modulation
    HWREG16(baseAddress + OFS_UCAxMCTLW) = 0;
}

void EUSCI_A_SPI_select4PinFunctionality (uint16_t baseAddress,
    uint16_t select4PinFunctionality
    )
{
  HWREG16(baseAddress + OFS_UCAxCTLW0) &= ~UCSTEM;
  HWREG16(baseAddress + OFS_UCAxCTLW0) |= select4PinFunctionality;
}

void EUSCI_A_SPI_changeMasterClock (uint16_t baseAddress,
    EUSCI_A_SPI_changeMasterClockParam *param)
{
    //Disable the USCI Module
    HWREG16(baseAddress + OFS_UCAxCTLW0) |= UCSWRST;

    HWREG16(baseAddress + OFS_UCAxBRW) =
        (uint16_t)(param->clockSourceFrequency / param->desiredSpiClock);

    //Reset the UCSWRST bit to enable the USCI Module
    HWREG16(baseAddress + OFS_UCAxCTLW0) &= ~(UCSWRST);
}

void EUSCI_A_SPI_initSlave (uint16_t baseAddress, EUSCI_A_SPI_initSlaveParam *param)
{
    //Disable USCI Module
    HWREG16(baseAddress + OFS_UCAxCTLW0)  |= UCSWRST;

    //Reset OFS_UCAxCTLW0 register
    HWREG16(baseAddress + OFS_UCAxCTLW0) &= ~(UCMSB +
                                            UC7BIT +
                                            UCMST +
                                            UCCKPL +
                                            UCCKPH +
                                            UCMODE_3
                                            );

    //Clock polarity, phase select, msbFirst, SYNC, Mode0
    HWREG16(baseAddress + OFS_UCAxCTLW0) |= (param->clockPhase +
                                            param->clockPolarity +
                                            param->msbFirst +
                                            UCSYNC +
                                            param->spiMode
                                            );
}

void EUSCI_A_SPI_changeClockPhasePolarity (uint16_t baseAddress,
    uint16_t clockPhase,
    uint16_t clockPolarity
    )
{
  //Disable the USCI Module
  HWREG16(baseAddress + OFS_UCAxCTLW0) |= UCSWRST;

  HWREG16(baseAddress + OFS_UCAxCTLW0) &= ~(UCCKPH + UCCKPL);

  HWREG16(baseAddress + OFS_UCAxCTLW0) |= (
        clockPhase +
        clockPolarity
          );

  //Reset the UCSWRST bit to enable the USCI Module
  HWREG16(baseAddress + OFS_UCAxCTLW0) &= ~(UCSWRST);
}

void EUSCI_A_SPI_transmitData ( uint16_t baseAddress,
    uint8_t transmitData
    )
{
    HWREG16(baseAddress + OFS_UCAxTXBUF) = transmitData;
}

uint8_t EUSCI_A_SPI_receiveData (uint16_t baseAddress)
{
    return ( HWREG16(baseAddress + OFS_UCAxRXBUF)) ;
}

void EUSCI_A_SPI_enableInterrupt (uint16_t baseAddress,
    uint16_t mask
    )
{
    HWREG16(baseAddress + OFS_UCAxIE) |= mask;
}

void EUSCI_A_SPI_disableInterrupt (uint16_t baseAddress,
    uint16_t mask
    )
{
    HWREG16(baseAddress + OFS_UCAxIE) &= ~mask;
}

uint8_t EUSCI_A_SPI_getInterruptStatus (uint16_t baseAddress,
    uint8_t mask
    )
{
    return ( HWREG16(baseAddress + OFS_UCAxIFG) & mask );
}

void EUSCI_A_SPI_clearInterrupt (uint16_t baseAddress,
    uint16_t mask
    )
{
    HWREG16(baseAddress + OFS_UCAxIFG) &=  ~mask;
}

void EUSCI_A_SPI_enable (uint16_t baseAddress)
{
    //Reset the UCSWRST bit to enable the USCI Module
    HWREG16(baseAddress + OFS_UCAxCTLW0) &= ~(UCSWRST);
}

void EUSCI_A_SPI_disable (uint16_t baseAddress)
{
    //Set the UCSWRST bit to disable the USCI Module
    HWREG16(baseAddress + OFS_UCAxCTLW0) |= UCSWRST;
}

uint32_t EUSCI_A_SPI_getReceiveBufferAddress (uint16_t baseAddress)
{
    return ( baseAddress + OFS_UCAxRXBUF );
}

uint32_t EUSCI_A_SPI_getTransmitBufferAddress (uint16_t baseAddress)
{
    return ( baseAddress + OFS_UCAxTXBUF );
}

uint16_t EUSCI_A_SPI_isBusy (uint16_t baseAddress)
{
    //Return the bus busy status.
    return (HWREG16(baseAddress + OFS_UCAxSTATW) & UCBUSY);
}


#endif
//*****************************************************************************
//
//! Close the doxygen group for eusci_a_spi_api
//! @}
//
//*****************************************************************************

//*****************************************************************************
//
// pmm.h - Driver for the PMM Module.
//
//*****************************************************************************

#ifndef __MSP430WARE_PMM_H__
#define __MSP430WARE_PMM_H__

#include "inc/hw_memmap.h"

#ifdef __MSP430_HAS_PMM_FRAM__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// The following are values that can be passed to the mask parameter for
// functions: PMM_clearInterrupt(), and PMM_getInterruptStatus() as well as
// returned by the PMM_getInterruptStatus() function.
//
//*****************************************************************************
#define PMM_BOR_INTERRUPT                                             PMMBORIFG
#define PMM_RST_INTERRUPT                                             PMMRSTIFG
#define PMM_POR_INTERRUPT                                             PMMPORIFG
#define PMM_SVSH_INTERRUPT                                              SVSHIFG
#define PMM_LPM5_INTERRUPT                                           PMMLPM5IFG
#define PMM_ALL                                                          (0xA7)

//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************

//*****************************************************************************
//
//! \brief Enables the high-side SVS circuitry
//!
//!
//! Modified bits of \b PMMCTL0 register.
//!
//! \return None
//
//*****************************************************************************
extern void PMM_enableSVSH(void);

//*****************************************************************************
//
//! \brief Disables the high-side SVS circuitry
//!
//!
//! Modified bits of \b PMMCTL0 register.
//!
//! \return None
//
//*****************************************************************************
extern void PMM_disableSVSH(void);

//*****************************************************************************
//
//! \brief Makes the low-dropout voltage regulator (LDO) remain ON when going
//! into LPM 3/4.
//!
//!
//! Modified bits of \b PMMCTL0 register.
//!
//! \return None
//
//*****************************************************************************
extern void PMM_turnOnRegulator(void);

//*****************************************************************************
//
//! \brief Turns OFF the low-dropout voltage regulator (LDO) when going into
//! LPM3/4, thus the system will enter LPM3.5 or LPM4.5 respectively
//!
//!
//! Modified bits of \b PMMCTL0 register.
//!
//! \return None
//
//*****************************************************************************
extern void PMM_turnOffRegulator(void);

//*****************************************************************************
//
//! \brief Calling this function will trigger a software Power On Reset (POR).
//!
//!
//! Modified bits of \b PMMCTL0 register.
//!
//! \return None
//
//*****************************************************************************
extern void PMM_trigPOR(void);

//*****************************************************************************
//
//! \brief Calling this function will trigger a software Brown Out Rest (BOR).
//!
//!
//! Modified bits of \b PMMCTL0 register.
//!
//! \return None
//
//*****************************************************************************
extern void PMM_trigBOR(void);

//*****************************************************************************
//
//! \brief Clears interrupt flags for the PMM
//!
//! \param mask is the mask for specifying the required flag
//!        Mask value is the logical OR of any of the following:
//!        - \b PMM_BOR_INTERRUPT - Software BOR interrupt
//!        - \b PMM_RST_INTERRUPT - RESET pin interrupt
//!        - \b PMM_POR_INTERRUPT - Software POR interrupt
//!        - \b PMM_SVSH_INTERRUPT - SVS high side interrupt
//!        - \b PMM_LPM5_INTERRUPT - LPM5 indication
//!        - \b PMM_ALL - All interrupts
//!
//! Modified bits of \b PMMCTL0 register and bits of \b PMMIFG register.
//!
//! \return None
//
//*****************************************************************************
extern void PMM_clearInterrupt(uint16_t mask);

//*****************************************************************************
//
//! \brief Returns interrupt status
//!
//! \param mask is the mask for specifying the required flag
//!        Mask value is the logical OR of any of the following:
//!        - \b PMM_BOR_INTERRUPT - Software BOR interrupt
//!        - \b PMM_RST_INTERRUPT - RESET pin interrupt
//!        - \b PMM_POR_INTERRUPT - Software POR interrupt
//!        - \b PMM_SVSH_INTERRUPT - SVS high side interrupt
//!        - \b PMM_LPM5_INTERRUPT - LPM5 indication
//!        - \b PMM_ALL - All interrupts
//!
//! \return Logical OR of any of the following:
//!         - \b PMM_BOR_INTERRUPT Software BOR interrupt
//!         - \b PMM_RST_INTERRUPT RESET pin interrupt
//!         - \b PMM_POR_INTERRUPT Software POR interrupt
//!         - \b PMM_SVSH_INTERRUPT SVS high side interrupt
//!         - \b PMM_LPM5_INTERRUPT LPM5 indication
//!         - \b PMM_ALL All interrupts
//!         \n indicating  the status of the selected  interrupt flags
//
//*****************************************************************************
extern uint16_t PMM_getInterruptStatus(uint16_t mask);

//*****************************************************************************
//
//! \brief Unlock LPM5
//!
//! LPMx.5 configuration is not locked and defaults to its reset condition.
//! Disable the GPIO power-on default high-impedance mode to activate
//! previously configured port settings.
//!
//!
//! \return None
//
//*****************************************************************************
extern void PMM_unlockLPM5(void);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif
#endif // __MSP430WARE_PMM_H__

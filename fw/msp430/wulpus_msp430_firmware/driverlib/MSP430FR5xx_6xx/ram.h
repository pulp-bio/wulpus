//*****************************************************************************
//
// ram.h - Driver for the RAM Module.
//
//*****************************************************************************

#ifndef __MSP430WARE_RAM_H__
#define __MSP430WARE_RAM_H__

#include "inc/hw_memmap.h"

#ifdef __MSP430_HAS_RC_FRAM__

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
// The following are values that can be passed to the sector parameter for
// functions: RAM_setSectorOff(), and RAM_getSectorState().
//
//*****************************************************************************
#define RAM_SECTOR0                                                      (0x00)
#define RAM_SECTOR1                                                      (0x01)
#define RAM_SECTOR2                                                      (0x02)
#define RAM_SECTOR3                                                      (0x03)

//*****************************************************************************
//
// The following are values that can be passed to the mode parameter for
// functions: RAM_setSectorOff() as well as returned by the
// RAM_getSectorState() function.
//
//*****************************************************************************
#define RAM_RETENTION_MODE                                               (0x00)
#define RAM_OFF_WAKEUP_MODE                                         (RCRS0OFF0)
#define RAM_OFF_NON_WAKEUP_MODE                                     (RCRS0OFF1)

//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************

//*****************************************************************************
//
//! \brief Set specified RAM sector off
//!
//! \param sector is specified sector to be set off.
//!        Valid values are:
//!        - \b RAM_SECTOR0
//!        - \b RAM_SECTOR1
//!        - \b RAM_SECTOR2
//!        - \b RAM_SECTOR3
//! \param mode is sector off mode
//!        Valid values are:
//!        - \b RAM_RETENTION_MODE
//!        - \b RAM_OFF_WAKEUP_MODE
//!        - \b RAM_OFF_NON_WAKEUP_MODE
//!
//! Modified bits of \b RCCTL0 register.
//!
//! \return None
//
//*****************************************************************************
extern void RAM_setSectorOff(uint8_t sector,
                             uint8_t mode);

//*****************************************************************************
//
//! \brief Get RAM sector ON/OFF status
//!
//! \param sector is specified sector
//!        Valid values are:
//!        - \b RAM_SECTOR0
//!        - \b RAM_SECTOR1
//!        - \b RAM_SECTOR2
//!        - \b RAM_SECTOR3
//!
//! \return One of the following:
//!         - \b RAM_RETENTION_MODE
//!         - \b RAM_OFF_WAKEUP_MODE
//!         - \b RAM_OFF_NON_WAKEUP_MODE
//!         \n indicating the status of the masked sectors
//
//*****************************************************************************
extern uint8_t RAM_getSectorState(uint8_t sector);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif
#endif // __MSP430WARE_RAM_H__

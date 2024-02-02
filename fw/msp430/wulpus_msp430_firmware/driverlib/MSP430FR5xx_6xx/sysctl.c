//*****************************************************************************
//
// sysctl.c - Driver for the sysctl Module.
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup sysctl_api sysctl
//! @{
//
//*****************************************************************************

#include "inc/hw_memmap.h"

#ifdef __MSP430_HAS_SYS__
#include "sysctl.h"

#include <assert.h>

void SysCtl_enableDedicatedJTAGPins (void)
{
    HWREG8(SYS_BASE + OFS_SYSCTL_L) |= SYSJTAGPIN;
}

uint8_t SysCtl_getBSLEntryIndication (void)
{
    if ( HWREG8(SYS_BASE + OFS_SYSCTL_L) & SYSBSLIND){
        return (SYSCTL_BSLENTRY_INDICATED) ;
    } else   {
        return (SYSCTL_BSLENTRY_NOTINDICATED) ;
    }
}

void SysCtl_enablePMMAccessProtect (void)
{
    HWREG8(SYS_BASE + OFS_SYSCTL_L) |= SYSPMMPE;
}

void SysCtl_enableRAMBasedInterruptVectors (void)
{
    HWREG8(SYS_BASE + OFS_SYSCTL_L) |= SYSRIVECT;
}

void SysCtl_disableRAMBasedInterruptVectors (void)
{
    HWREG8(SYS_BASE + OFS_SYSCTL_L) &= ~(SYSRIVECT);
}

void SysCtl_initJTAGMailbox (uint8_t mailboxSizeSelect,
    uint8_t autoClearInboxFlagSelect)
{
    HWREG8(SYS_BASE + OFS_SYSJMBC_L) &= ~(JMBCLR1OFF + JMBCLR0OFF + JMBMODE);
    HWREG8(SYS_BASE + OFS_SYSJMBC_L) |=
        mailboxSizeSelect + autoClearInboxFlagSelect;
}

uint8_t SysCtl_getJTAGMailboxFlagStatus (uint8_t mailboxFlagMask)
{
    return ( HWREG8(SYS_BASE + OFS_SYSJMBC_L) & mailboxFlagMask);
}

void SysCtl_clearJTAGMailboxFlagStatus (uint8_t mailboxFlagMask)
{
    HWREG8(SYS_BASE + OFS_SYSJMBC_L) &= ~(mailboxFlagMask);
}

uint16_t SysCtl_getJTAGInboxMessage16Bit (uint8_t inboxSelect)
{
    return ( HWREG16(SYS_BASE + OFS_SYSJMBI0 + inboxSelect) );
}

uint32_t SysCtl_getJTAGInboxMessage32Bit (void)
{
    uint32_t JTAGInboxMessageLow = HWREG16(SYS_BASE + OFS_SYSJMBI0);
    uint32_t JTAGInboxMessageHigh = HWREG16(SYS_BASE + OFS_SYSJMBI1);

    return ( (JTAGInboxMessageHigh << 16) + JTAGInboxMessageLow );
}

void SysCtl_setJTAGOutgoingMessage16Bit (uint8_t outboxSelect,
    uint16_t outgoingMessage)
{
    HWREG16(SYS_BASE + OFS_SYSJMBO0 + outboxSelect) = outgoingMessage;
}

void SysCtl_setJTAGOutgoingMessage32Bit (uint32_t outgoingMessage)
{
    HWREG16(SYS_BASE + OFS_SYSJMBO0) = (outgoingMessage);
    HWREG16(SYS_BASE + OFS_SYSJMBO1) = (outgoingMessage >> 16);
}


#endif
//*****************************************************************************
//
//! Close the doxygen group for sysctl_api
//! @}
//
//*****************************************************************************

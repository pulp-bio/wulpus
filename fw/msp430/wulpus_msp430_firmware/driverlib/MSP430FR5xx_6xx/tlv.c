//*****************************************************************************
//
// tlv.c - Driver for the tlv Module.
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup tlv_api tlv
//! @{
//
//*****************************************************************************

#include "inc/hw_memmap.h"

#ifdef __MSP430_HAS_TLV__
#include "tlv.h"

#include <assert.h>

void TLV_getInfo(uint8_t tag,
        uint8_t instance,
        uint8_t *length,
        uint16_t **data_address
        )
{
    // TLV Structure Start Address
    char *TLV_address = (char *)TLV_START;

    while((TLV_address < (char *)TLV_END)
            && ((*TLV_address != tag) || instance)   // check for tag and instance
            && (*TLV_address != TLV_TAGEND))         // do range check first
    {
        if (*TLV_address == tag)
        {
            // repeat till requested instance is reached
            instance--;
        }
        // add (Current TAG address + LENGTH) + 2
        TLV_address += *(TLV_address + 1) + 2;
    }

    // Check if Tag match happened..
    if (*TLV_address == tag)
    {
        // Return length = Address + 1
        *length = *(TLV_address + 1);
        // Return address of first data/value info = Address + 2
        *data_address = (uint16_t *)(TLV_address + 2);
    }
    // If there was no tag match and the end of TLV structure was reached..
    else
    {
        // Return 0 for TAG not found
        *length = 0;
        // Return 0 for TAG not found
        *data_address = 0;
    }
}

uint16_t TLV_getDeviceType()
{
    uint16_t *pDeviceType = (uint16_t *)TLV_DEVICE_ID_0;
    // Return Value from TLV Table
    return pDeviceType[0];
}

uint16_t TLV_getMemory(uint8_t instance)
{
    uint8_t *pPDTAG;
    uint8_t bPDTAG_bytes;
    uint16_t count;

    // set tag for word access comparison
    instance *= 2;

    // TLV access Function Call
    // Get Peripheral data pointer
    TLV_getInfo(TLV_PDTAG,
            0,
            &bPDTAG_bytes,
            (uint16_t **)&pPDTAG
            );
    if (pPDTAG != 0)
    {
        for (count = 0; count <= instance; count += 2)
        {
            if (pPDTAG[count] == 0)
            {
                // Return 0 if end reached
                return 0;
            }
            if (count == instance)
            {
                return (pPDTAG[count] | pPDTAG[count+1]<<8);
            }
        }   // for count
    }   // pPDTAG != 0

    // Return 0: not found
    return 0;
}

uint16_t TLV_getPeripheral(uint8_t tag,
        uint8_t instance
        )
{
    uint8_t *pPDTAG;
    uint8_t bPDTAG_bytes;
    uint16_t count = 0;
    uint16_t pcount = 0;

    // Get Peripheral data pointer
    TLV_getInfo(TLV_PDTAG,
            0,
            &bPDTAG_bytes,
            (uint16_t **)&pPDTAG
            );
    if (pPDTAG != 0)
    {
        // read memory configuration from TLV to get offset for Peripherals
        while (TLV_getMemory(count))
        {
            count++;
        }
        // get number of Peripheral entries
        pcount = pPDTAG[count * 2 + 1];
        // inc count to first Periperal
        count++;
        // adjust point to first address of Peripheral
        pPDTAG += count*2;
        // set counter back to 0
        count = 0;
        // align pcount for work comparision
        pcount *= 2;

        // TLV access Function Call
        for (count = 0; count <= pcount; count += 2)
        {
            if (pPDTAG[count+1] == tag)
            {
                // test if required Peripheral is found
                if (instance > 0)
                {
                    // test if required instance is found
                    instance--;
                }
                else
                {
                    // Return found data
                    return (pPDTAG[count] | pPDTAG[count + 1] << 8);
                }
            }   // pPDTAG[count+1] == tag
        }   // for count
    }   // pPDTAG != 0

    // Return 0: not found
    return 0;
}

uint8_t TLV_getInterrupt(uint8_t tag)
{
    uint8_t *pPDTAG;
    uint8_t bPDTAG_bytes;
    uint16_t count = 0;
    uint16_t pcount = 0;

    // Get Peripheral data pointer
    TLV_getInfo(TLV_PDTAG,
            0,
            &bPDTAG_bytes,
            (uint16_t **)&pPDTAG
            );
    if (pPDTAG != 0)
    {
        // read memory configuration from TLV to get offset for Peripherals
        while (TLV_getMemory(count))
        {
            count++;
        }

        pcount = pPDTAG[count * 2 + 1];
        // inc count to first Periperal
        count++;
        // adjust point to first address of Peripheral
        pPDTAG += (pcount + count) * 2;
        // set counter back to 0
        count = 0;

        // TLV access Function Call
        for (count = 0; count <= tag; count += 2)
        {
            if (pPDTAG[count] == 0)
            {
                // Return 0: not found/end of table
                return 0;
            }
            if (count == tag)
            {
                // Return found data
                return (pPDTAG[count]);
            }
        }   // for count
    }   // pPDTAG != 0

    // Return 0: not found
    return 0;
}

#endif
//*****************************************************************************
//
//! Close the doxygen group for tlv_api
//! @}
//
//*****************************************************************************

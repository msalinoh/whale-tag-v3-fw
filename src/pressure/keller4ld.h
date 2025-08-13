//-----------------------------------------------------------------------------
// Project:      CETI Tag Electronics
// Version:      Refer to _versioning.h
// Copyright:    Cummings Electronics Labs, Harvard University Wood Lab,
//               MIT CSAIL
// Contributors: Matt Cummings, Peter Malkin, Joseph DelPreto,
//               Michael Salino-Hugg, [TODO: Add other contributors here]
// Description: PDevice driver for Keller 4LD pressure transmitter
//-----------------------------------------------------------------------------
#ifndef __CETI_WHALE_TAG_HAL_KELLER_4LD__
#define __CETI_WHALE_TAG_HAL_KELLER_4LD__

#include "stm32u5xx_hal.h"

#include <stdint.h>

typedef struct {
    uint8_t status;
    uint16_t pressure;
    uint16_t temperature;
} Keller4LD_Measurement;

HAL_StatusTypeDef keller4ld_request_measurement(void);
HAL_StatusTypeDef keller4ld_read_status(uint8_t *pStatus);
HAL_StatusTypeDef keller4ld_read_measurement(Keller4LD_Measurement *pData);

#endif // __CETI_WHALE_TAG_HAL_KELLER_4LD__

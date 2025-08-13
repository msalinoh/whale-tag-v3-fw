//-----------------------------------------------------------------------------
// Project:      CETI Tag Electronics
// Version:      Refer to _versioning.h
// Copyright:    Cummings Electronics Labs, Harvard University Wood Lab,
//               MIT CSAIL
// Contributors: Matt Cummings, Peter Malkin, Joseph DelPreto,
//               Michael Salino-Hugg, [TODO: Add other contributors here]
// Description: PDevice driver for Keller 4LD pressure transmitter
//-----------------------------------------------------------------------------

#include "keller4ld.h"


#include "main.h"
#include "i2c.h"

extern I2C_HandleTypeDef KELLER_hi2c;

typedef enum {
    KELLER4LD_MODE_DELAY,
    KELLER4LD_MODE_POLL,
    KELLER4LD_MODE_INTERRUPT,
} Keller4ldMode;

#define KELLER4LD_DEVICE_ADDR_DEFAULT 0x40

#define KELLER4LD_REG_SLAVE_ADDRESS 0x42

#define KELLER4LD_CMD_COMMAND_MODE 0xA9
#define KELLER4LD_CMD_REQUEST_MEASUREMENT 0xAC

HAL_StatusTypeDef keller4ld_request_measurement(void) {
    uint8_t req = KELLER4LD_CMD_REQUEST_MEASUREMENT;
	return HAL_I2C_Master_Transmit(&KELLER_hi2c, (KELLER4LD_DEVICE_ADDR_DEFAULT << 1), &req, 1, 1);
}

HAL_StatusTypeDef keller4ld_read_status(uint8_t *pStatus){
    return HAL_I2C_Master_Receive(&KELLER_hi2c, (KELLER4LD_DEVICE_ADDR_DEFAULT << 1), pStatus, 1, 8);
}

HAL_StatusTypeDef keller4ld_read_measurement(Keller4LD_Measurement *pData){
    uint8_t raw[5] = {};
    HAL_I2C_Master_Receive(&KELLER_hi2c, (KELLER4LD_DEVICE_ADDR_DEFAULT << 1), raw, sizeof(raw), 8);
    pData->status = raw[0];
    if (((pData->status >> 6) & 0b11) != 0b01) {
        // ToDo: invalid packet
    }
    pData->pressure = (((uint16_t)raw[1] << 8) | (uint16_t)raw[2]);
    pData->temperature = (((uint16_t)raw[3] << 8) | (uint16_t)raw[4]);
    return HAL_OK;
}

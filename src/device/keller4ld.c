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
#include "stm32u5xx_hal.h"

#include <i2c.h>

#include <stdint.h>
// #include <hal_stm32u5xx.h>

#define KELLER_4LD_REQUEST_WAIT_TIME_US (8000)

// Keller 4LD Pressure Sensor 200 bar
// Reference pressure is a 1 bar abs
#define PRESSURE_MIN 0   // bar
#define PRESSURE_MAX 200 // bar

typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint8_t valid;
    uint8_t raw[5];
    uint16_t latest_sample;
} Keller4ldDescription;

static Keller4ldDescription self;

typedef enum {
    KELLER_4LD_CMD_REQUEST_MEASUREMENT = 0xAC,
} Keller4ldCommand;

HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c) {

}

void keller4ld_dma_cmplt_callback(void) {

}

// Keller EOC callback
void keller4ld_eoc_callback(uint16_t GPIO_Pin) {
    // use DMA callback
    HAL_I2C_Master_Receive_DMA(self.hi2c, (PRESSURE_I2C_DEV_ADDR << 1), self.raw, sizeof(self.raw));
}

int keller4ld_get_measurement(uint16_t *pPressure, uint16_t *pTemp, uint32_t Timeout) {
    uint8_t req = 0xAC;
    HAL_StatusTypeDef i2c_result = HAL_OK;

    i2c_result = HAL_I2C_Master_Transmit(self.hi2c, (PRESSURE_I2C_DEV_ADDR << 1), &req, 1, Timeout);
    HAL_Delay(8);
    i2c_result = HAL_I2C_Master_Transmit(self.hi2c, (PRESSURE_I2C_DEV_ADDR << 1), self.raw, 5, Timeout);
    // parse status byte to verify validity
    uint8_t status = self.raw[0];
    if ((status & 0b11000100) != 0b01000000) { // invalid packet
//        return WT_RESULT(WT_DEV_PRESSURE, WT_ERR_PRESSURE_INVALID_RESPONSE);
    }

    if (status & 0b00100000) { // device is busy
//        return WT_RESULT(WT_DEV_PRESSURE, WT_ERR_PRESSURE_BUSY);
    }

    // packet is ok, parse data
    if (pPressure != NULL) {
        *pPressure = (((uint16_t)self.raw[1]) << 8) | ((uint16_t)self.raw[2]);
    }

    if (pTemp != NULL) {
        *pTemp = (((uint16_t)self.raw[3]) << 8) | ((uint16_t)self.raw[4]);
    }
    return 0;
}

HAL_StatusTypeDef keller4ld_get_measurement_IT(void) {
    uint8_t req = 0xAC;
    //register eoc callback
    return HAL_I2C_Master_Transmit_DMA(self.hi2c, (PRESSURE_I2C_DEV_ADDR << 1), &req, 1);
}

int pressure_get_measurement(uint16_t *pPressure, uint16_t *pTemp) {
    return keller4ld_get_measurement(pPressure, pTemp, 1000);
}

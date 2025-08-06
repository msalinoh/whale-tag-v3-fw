#include "ads1219.h"

#include "main.h"
#include "gpio.h"
#include "i2c.h"

#define ADS1219_CMD_RESET 0x06
#define ADS1219_CMD_START 0x08
#define ADS1219_CMD_POWERDOWN 0x02
#define ADS1219_CMD_RREG 0x10

#define _LSHIFT(x, s, w) (((x) & ((1 << w) - 1)) << s)

static void __ads1219_write_config_register(uint8_t value) {
    HAL_I2C_Mem_Write(&ECG_hi2c, 0x68, 0x40, 1, &value, 1, 5);
}

static void __ads1219_write_cmd(uint8_t cmd) {
    HAL_I2C_Master_Transmit(&ECG_hi2c, 0x68, &cmd, 1, 5);
}

void ads1219_apply_configuration(const ADS1219_Configuration *configuration) {
    uint8_t config = _LSHIFT((uint8_t)configuration->vref, 0 , 1)
        | _LSHIFT((uint8_t)configuration->mode, 1, 1)
        | _LSHIFT((uint8_t)configuration->data_rate, 2, 2)
        | _LSHIFT((uint8_t)configuration->gain, 4, 1)
        | _LSHIFT((uint8_t)configuration->mux, 5, 3)
        ;
    __ads1219_write_config_register(config);
}

void ads1219_start(void) {
    __ads1219_write_cmd(ADS1219_CMD_START);
}

void ads1219_read_data_raw(int32_t *reading) {
    uint8_t raw[3] = {};
    __ads1219_write_cmd(ADS1219_CMD_RREG);
    HAL_I2C_Master_Receive(&ECG_hi2c, 0x68, raw, 3, 1);
     // Parse the data bytes into as 24 bytes of big-endian data.
    int32_t result_data = (((int32_t)raw[0]) << 24) | (((int32_t)raw[1]) << 16) | (((int32_t)raw[2]) << 8);
    result_data >>= 8; // convert to 24-bit value in 32-bit storage
    if( reading != NULL ) {
        *reading = result_data;
    }
}

void ads1219_reset(void) {
    __ads1219_write_cmd(ADS1219_CMD_RESET);
}

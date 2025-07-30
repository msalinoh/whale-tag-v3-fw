#include "device/max17320.h"

#include "util/timing.h"

typedef struct {
    time_t time_us;
    uint32_t error;
    uint16_t cell_voltage_raw[2];
    uint16_t cell_temperature_raw[2];
    uint16_t current_raw;
    uint16_t state_of_charge_raw;
    uint16_t status;
    uint16_t protection_alert;
} CetiBatteryRawSample;

CetiBatteryRawSample s_battery_sample = {};

void acq_battery_get_sample(void) {
    // create sample
    s_battery_sample.time_us = rtc_get_epoch_us();
    s_battery_sample.error = 0;
    // if (shm_battery->error == WT_OK) {
        // shm_battery->error = max17320_get_cell_voltage_v(0, &shm_battery->cell_voltage_v[0]);
    // }
    // if (shm_battery->error == WT_OK) {
    //     shm_battery->error = max17320_get_cell_voltage_v(1, &shm_battery->cell_voltage_v[1]);
    // }
    // if (shm_battery->error == WT_OK) {
    //     shm_battery->error = max17320_get_current_mA(&shm_battery->current_mA);
    // }
    for (int i = 0; i < MAX17320_CELL_COUNT; i++) {
        if (s_battery_sample.error != 0) {
            break;
        }
        max17320_get_cell_temperature_raw(i, &s_battery_sample.cell_temperature_raw[i]);
    }
    // if (shm_battery->error == WT_OK) {
    //     shm_battery->error = max17320_get_state_of_charge(&shm_battery->state_of_charge);
    // }
    // if (shm_battery->error == WT_OK) {
    //     shm_battery->error = max17320_read(MAX17320_REG_STATUS, &shm_battery->status);
    // }
    // if (shm_battery->error == WT_OK) {
    //     shm_battery->error = max17320_read(MAX17320_REG_PROTALRT, &shm_battery->protection_alert);
    // }

    // push semaphore to indicate to user applications that new data is available
    // sem_post(sem_battery_data_ready);

    // clear protection alert flags and status flags
    // if (shm_battery->error == WT_OK) {
    //     shm_battery->error = max17320_write(MAX17320_REG_PROTALRT, 0x0000);
    // }
    // if (shm_battery->error == WT_OK) {
    //     shm_battery->error = max17320_write(MAX17320_REG_STATUS, 0x0000);
    // }
}

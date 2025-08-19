/*****************************************************************************
 *   @file      battery/acq_battery.h
 *   @brief     Battery sample acquisition and buffering code
 *   @project   Project CETI
 *   @copyright Harvard University Wood Lab
 *   @authors   Michael Salino-Hugg, [TODO: Add other contributors here]
 *****************************************************************************/
#ifndef CETI_ACQ_BATTERY_H
#define CETI_ACQ_BATTERY_H

#include <stdint.h>
#include "timing.h"

typedef struct {
    time_t time_us;
    uint32_t error;
    double cell_voltage_v[2];
    double cell_temperature_c[2];
    double current_mA;
    double state_of_charge_percent;
    uint16_t status;
    uint16_t protection_alert;
} CetiBatterySample;

void acq_battery_init(void);
void acq_battery_start(void);
void acq_battery_stop(void);
const CetiBatterySample* acq_battery_get_next_sample(void);
void acq_pressure_get_next_buffer_range(uint8_t **ppBuffer, size_t *pSize);
void acq_battery_peak_latest_sample(CetiBatterySample *pSample);
#endif // CETI_ACQ_BATTERY_H
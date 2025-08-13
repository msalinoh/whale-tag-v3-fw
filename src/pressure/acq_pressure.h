/*****************************************************************************
 *   @file      acq_pressure.h
 *   @brief     pressure acquisition code.
 *   @project   Project CETI
 *   @copyright Harvard University Wood Lab
 *   @authors   Michael Salino-Hugg, [TODO: Add other contributors here]
 *****************************************************************************/
#ifndef CETI_ACQ_PRESSURE_H
#define CETI_ACQ_PRESSURE_H

#include "keller4ld.h"
#include "timing.h"


typedef struct {
    time_t timestamp_us;
    Keller4LD_Measurement data;
} CetiPressureSample;

void acq_pressure_EXTI_cb(void);
void acq_pressure_enable(void);
void acq_pressure_disable(void);

#endif // CETI_ACQ_PRESSURE_H

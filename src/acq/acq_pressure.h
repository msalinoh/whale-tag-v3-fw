
/*****************************************************************************
 *   @file   acq/acq_pressure.h
 *   @brief  ecg acquisition code. Note this code gathers pressure data 
 *           into RAM, but does not perform any analysis, transformation, or
 *           storage of said data.
 *   @author Michael Salino-Hugg (msalinohugg@seas.harvard.edu)
 *****************************************************************************/
#ifndef CETI_ACQ_PRESSURE_H
#define CETI_ACQ_PRESSURE_H

#include "device/keller4ld.h"
#include "util/timing.h"


typedef struct {
    time_t timestamp_us;
    Keller4LD_Measurement data;
} CetiPressureSample;

void acq_pressure_EXTI_cb(void);
void acq_pressure_enable(void);
void acq_pressure_disable(void);

#endif // CETI_ACQ_PRESSURE_H

/*****************************************************************************
 *   @file      acq_pressure.c
 *   @brief     pressure acquisition code.
 *   @project   Project CETI
 *   @copyright Harvard University Wood Lab
 *   @authors   Michael Salino-Hugg, [TODO: Add other contributors here]
 *****************************************************************************/
#include "acq_pressure.h"

#include "main.h"
#include "tim.h"

#include <string.h>

extern TIM_HandleTypeDef PRESSURE_htim;

#define ACQ_PRESSURE_BUFFER_SIZE (8)

static CetiPressureSample pressure_sample_buffer[ACQ_PRESSURE_BUFFER_SIZE]; //minute long buffer
static uint16_t s_acq_pressure_write_position = 0;
static uint16_t s_acq_pressure_latest_position = 0;
static uint16_t s_acq_pressure_read_position = 0;

// TIM3-> <request measurement> -> <EOC EXTI> -> <read measurement start> -> <read measurement complete interrupt>

void acq_pressure_EXTI_cb(void) {
    CetiPressureSample * p_sample = &pressure_sample_buffer[s_acq_pressure_write_position];
    p_sample->timestamp_us = rtc_get_epoch_us();
    int result = keller4ld_read_measurement_it(&p_sample->data);

    s_acq_pressure_latest_position = s_acq_pressure_write_position;
    s_acq_pressure_write_position = (s_acq_pressure_write_position + 1) % ACQ_PRESSURE_BUFFER_SIZE;
    if (s_acq_pressure_write_position == s_acq_pressure_read_position) {
        // ToDo: Handle overflow
    }
}

static void __acq_pressure_timer_complete_cb(TIM_HandleTypeDef *htim) {
	int result = keller4ld_request_measurement_it();
}

// Note: this method is currently kinda unsafe since the buffer could be overwritten once pointer is returned
// however it is faster than copying the memory to another location
const CetiPressureSample *acq_pressure_get_next_sample(void) {
    if (s_acq_pressure_read_position == s_acq_pressure_write_position) {
        return NULL;
    }

    const CetiPressureSample *next_sample = &pressure_sample_buffer[s_acq_pressure_read_position];
    s_acq_pressure_read_position = (s_acq_pressure_read_position + 1) % ACQ_PRESSURE_BUFFER_SIZE;
    return next_sample;
}

void acq_pressure_get_next_buffer_range(void **ppBuffer, size_t *pSize) {
    if ((ppBuffer == NULL) || (pSize == NULL)){
        //ToDo: bad params handler
        return;
    }

    // calculate buffer range and size
    uint16_t end_index = s_acq_pressure_write_position;
    if (end_index < s_acq_pressure_read_position) {
        end_index = ACQ_PRESSURE_BUFFER_SIZE; // only go to end if 
    }
    *ppBuffer = &pressure_sample_buffer[s_acq_pressure_read_position];
    *pSize = (end_index - s_acq_pressure_read_position)*sizeof(CetiPressureSample);
    
    // update read position
    s_acq_pressure_read_position = end_index % ACQ_PRESSURE_BUFFER_SIZE;
}

void acq_pressure_peak_latest_sample(CetiPressureSample *pSample) {
    memcpy(pSample, &pressure_sample_buffer[s_acq_pressure_latest_position], sizeof(CetiPressureSample));
    return;
}

void acq_pressure_init(void) {
    // configure timer to 1 second
    MX_TIM3_Init();

    // setup timer callback
    HAL_TIM_RegisterCallback(&PRESSURE_htim, HAL_TIM_PERIOD_ELAPSED_CB_ID, __acq_pressure_timer_complete_cb);
    return;
}

void acq_pressure_start(void) {
    HAL_TIM_Base_Start_IT(&PRESSURE_htim);
}

void acq_pressure_stop(void) {
    HAL_TIM_Base_Stop_IT(&PRESSURE_htim);
}

void acq_pressure_disable(void) {
    acq_pressure_stop();
    HAL_TIM_UnRegisterCallback(&PRESSURE_htim, HAL_TIM_PERIOD_ELAPSED_CB_ID);
    HAL_TIM_Base_DeInit(&PRESSURE_htim);
}

void acq_pressure_flush(void) {
}

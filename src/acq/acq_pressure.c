#include "acq_pressure.h"

#include "main.h"
#include "tim.h"

extern TIM_HandleTypeDef PRESSURE_htim;

static CetiPressureSample pressure_sample_buffer[2*60]; //minute long buffer
static int s_acq_pressure_flush_ready = 0;
static uint16_t s_acq_pressure_write_position = 0;
static uint16_t s_acq_pressure_read_position = 0;

void acq_pressure_EXTI_cb(void) {
    CetiPressureSample * p_sample = &pressure_sample_buffer[s_acq_pressure_write_position];
    p_sample->timestamp_us = rtc_get_epoch_us();
    int result = keller4ld_read_measurement(&p_sample->data);
    
    // ToDo: Errory handle
    s_acq_pressure_write_position = (s_acq_pressure_write_position + 1) % (2*60);
    if (s_acq_pressure_write_position == s_acq_pressure_read_position) {
        // ToDo: Handle overflow
    }
    if ((s_acq_pressure_write_position == 0) || (s_acq_pressure_write_position == 60)){
    	s_acq_pressure_flush_ready = 1;
    }
}

static void __acq_pressure_timer_complete_cb(TIM_HandleTypeDef *htim) {
	int result = keller4ld_request_measurement();
}

void acq_pressure_enable(void) {
    // configure timer to 1 second
    MX_TIM3_Init();
    HAL_TIM_RegisterCallback(&PRESSURE_htim, HAL_TIM_PERIOD_ELAPSED_CB_ID, __acq_pressure_timer_complete_cb);
    HAL_TIM_Base_Start_IT(&PRESSURE_htim);
}

void acq_pressure_disable(void) {
     HAL_TIM_UnRegisterCallback(&PRESSURE_htim, HAL_TIM_PERIOD_ELAPSED_CB_ID);
}

void acq_pressure_flush(void) {
}
#include "device/keller4ld.h"

typedef struct {
    // timestamp
    // status
    uint16_t pressure_raw; // pressure_raw
    uint16_t temperature_raw; // temperature_raw
} CetiPressureSample;

static CetiPressureSample pressure_sample_buffer[2][60]; //minute long buffer
static uint16_t s_acq_pressure_page = 0;
static uint16_t s_acq_pressure_position = 0;

static void s__sample_complete_callback(void) {
    CetiPressureSample *current_sample = &pressure_sample_buffer[s_acq_pressure_page][s_acq_pressure_position];
    
    current_sample->pressure_raw = keller4ld_get_pressure_raw();
    current_sample->temperature_raw = keller4ld_get_temperature_raw();
    
}

static void s__iniate_sensor_reading(void) {
    
}

void acq_pressure_enable(void){
    // setup keller
    keller4ld_init();
    
    // setup keller_sample_complete_callback
    // enable 1 second timer interrupt to initiate sampling
}

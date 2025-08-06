#include "log_pressure.h"

#include <stdint.h>

#include <app_filex.h>

#include "acq/acq_pressure.h"

typedef enum {
    PRESSURE_FORMAT_BIN,
    PRESSURE_FORMAT_BIN_COMPRESSED,
    PRESSURE_FORMAT_CSV,
    PRESSURE_FORMAT_CSV_COMPRESSED,
} LogPressureFormat;

#define LOG_PRESSURE_FORMAT PRESSURE_FORMAT_CSV

#define PRESSURE_FILENAME "data_pressure.csv"

extern FX_MEDIA sdio_disk;
FX_FILE pressure_file = {};


static void log_pressure_open_csv_file(void) {
    /* Create/open audio file */
    UINT fx_result = FX_ACCESS_ERROR;
    fx_result = fx_file_create(&sdio_disk, PRESSURE_FILENAME);
    int new_file = 0;
    if ((fx_result != FX_SUCCESS) && (fx_result != FX_ALREADY_CREATED)) {
        Error_Handler();
    } else if ((fx_result != FX_ALREADY_CREATED)) {
        new_file = 1;
        CETI_LOG("Created new pressure file \"%s\"", PRESSURE_FILENAME);
    }
    
    fx_result = fx_file_open(&sdio_disk, &pressure_file, PRESSURE_FILENAME, FX_OPEN_FOR_WRITE);
    if (fx_result != FX_SUCCESS) {
        Error_Handler();
    }
    CETI_LOG("Opened pressure file \"%s\"", PRESSURE_FILENAME);

    fx_result = fx_file_relative_seek(&pressure_file, 0, FX_SEEK_END);

    if (new_file) {
        // write header
//        fx_file_write(pressure_file, "Timestamp [uS], Notes, Pressure [Bar], Temperature [C]\n\r", );
    }
    // Set "write complete" "callback"
    // fx_result = fx_file_write_notify_set(&pressure_file, audio_SDWriteComplete);
    // if(fx_result != FX_SUCCESS){
    //     Error_Handler();
    // }
}

static void log_pressure_sample_to_csv(const CetiPressureSample*sample, uint8_t * pBuffer, size_t buffer_size) {    
}

void log_pressure_enable(void) {
    
}

void log_pressure_disable(void) {
    // ToDo: stop pressure acquisition
    // ToDo: flush partial data to SD card
    // ToDo: close pressure file
}

void log_pressure_task(void) {
    // ToDo: check if pressure is ready to be flushed {
        // ToDo: process data buffer
        // ToDo: flush data
    //}
}

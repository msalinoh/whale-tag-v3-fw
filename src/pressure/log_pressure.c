/*****************************************************************************
 *   @file      log_pressure.c
 *   @brief     pressure processing and storing code.
 *   @project   Project CETI
 *   @copyright Harvard University Wood Lab
 *   @authors   Michael Salino-Hugg, [TODO: Add other contributors here]
 *****************************************************************************/
#include "log_pressure.h"
#include "acq_pressure.h"

#include "log/log_syslog.h"

#include <stdint.h>
#include <stdio.h>

#include <app_filex.h>


#define PRESSURE_FORMAT_BIN (0)    
#define PRESSURE_FORMAT_CSV (1)

#define LOG_PRESSURE_FORMAT PRESSURE_FORMAT_CSV


#define LOG_PRESSURE_ENCODE_BUFFER_FLUSH_THRESHOLD (3*1024)
#define LOG_PRESSURE_ENCODE_BUFFER_SIZE (LOG_PRESSURE_ENCODE_BUFFER_FLUSH_THRESHOLD + 512)

uint8_t log_pressure_encode_buffer[LOG_PRESSURE_ENCODE_BUFFER_SIZE];

extern FX_MEDIA sdio_disk;
FX_FILE pressure_file = {};

#define PRESSURE_FILENAME "data_pressure.csv"
char *log_pressure_csv_header =
    "Timestamp [us]"
    ", Notes"
    ", Pressure [bar]"
    ", Temperature [C]"
    "\n"
;

static void log_pressure_open_file(void) {
    /* Create/open presure file */
    UINT fx_result = FX_ACCESS_ERROR;
    fx_result = fx_file_create(&sdio_disk, PRESSURE_FILENAME);
    int is_new_file = 0;
    if ((fx_result != FX_SUCCESS) && (fx_result != FX_ALREADY_CREATED)) {
         Error_Handler();
    } else if ((fx_result != FX_ALREADY_CREATED)) {
        is_new_file = 1;
        CETI_LOG("Created new pressure file \"%s\"", PRESSURE_FILENAME);
    }
    
    fx_result = fx_file_open(&sdio_disk, &pressure_file, PRESSURE_FILENAME, FX_OPEN_FOR_WRITE);
    if (fx_result != FX_SUCCESS) {
         Error_Handler();
    }
    CETI_LOG("Opened pressure file \"%s\"", PRESSURE_FILENAME);

    if (!is_new_file) {
        return;
    }

    fx_result = fx_file_write(&pressure_file, log_pressure_csv_header, strlen(log_pressure_csv_header));
    if (fx_result != FX_SUCCESS) {
        // ToDo: error handling
        Error_Handler();
    }
}

static size_t log_pressure_sample_to_csv(const CetiPressureSample* pSample, uint8_t * pBuffer, size_t buffer_len) {  
    uint8_t offset = 0;
    offset += snprintf((char *)&pBuffer[offset], buffer_len - offset, "%lld", pSample->timestamp_us);

    /* ToDo: note conversion */
    offset += snprintf((char *)&pBuffer[offset], buffer_len - offset, ", ");
    // offset += snprintf(&pBuffer[offset], buffer_len - offset, ", %s", pSample->error);

    offset += snprintf((char *)&pBuffer[offset], buffer_len - offset, ", %.3f", acq_pressure_raw_to_pressure_bar(pSample->data.pressure));
    offset += snprintf((char *)&pBuffer[offset], buffer_len - offset, ", %.3f", acq_pressure_raw_to_temperature_c(pSample->data.temperature));
    offset += snprintf((char *)&pBuffer[offset], buffer_len - offset, "\n");
    return offset;  
}

void log_pressure_init(void) {
    // create pressure files
    log_pressure_open_file();
}

void log_pressure_task(void) {
    size_t encoded_bytes = 0;
    const CetiPressureSample* pSample = acq_pressure_get_next_sample();
    while (pSample != NULL) {
        encoded_bytes += log_pressure_sample_to_csv(pSample, &log_pressure_encode_buffer[encoded_bytes], LOG_PRESSURE_ENCODE_BUFFER_SIZE - encoded_bytes);

        // flush encode buffer if it is almost full
        if (encoded_bytes > LOG_PRESSURE_ENCODE_BUFFER_FLUSH_THRESHOLD) {
            UINT fx_result = fx_file_write(&pressure_file, log_pressure_encode_buffer, encoded_bytes);
            if (fx_result != FX_SUCCESS) {
                //  ToDo: Handle Errors
            }
            encoded_bytes = 0;
        }
        pSample = acq_pressure_get_next_sample();
    }
    
    //flush encode buffer
    if (encoded_bytes != 0) {
        // Flush buffer to SD card
        UINT fx_result = fx_file_write(&pressure_file, log_pressure_encode_buffer, encoded_bytes);
        if (fx_result != FX_SUCCESS) {
            //  ToDo: Handle Errors
        }
    }
}

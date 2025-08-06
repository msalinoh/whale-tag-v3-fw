// local files
#include "log_audio.h"

#include "acq/acq_audio.h"
#include "log/log_syslog.h"
#include "util/timing.h"

// stm libraries
#include <app_filex.h>
#include <main.h>

// std libraries
#include <stdint.h>
#include <stdio.h>
#include <time.h>

typedef enum {
    AUDIO_LOG_RAW,
    AUDIO_LOG_FLAC,
} AudioLogTypes;

#define AUDIO_LOG_TYPE AUDIO_LOG_RAW
static uint8_t s_log_audio_enabled = 0;
static int s_audio_log_page = 0;
static uint8_t sd_card_busy = 0;

static time_t s_audio_start_time_us;

extern FX_MEDIA sdio_disk;
FX_FILE audio_file = {};

void audio_SDWriteComplete(FX_FILE *file) {
	sd_card_busy = 0;
	s_audio_log_page ^= 1; // advance read head
}

#if AUDIO_LOG_TYPE == AUDIO_LOG_RAW
static void log_audio_create_raw_file(void) {
    /* Create file based on RTC time */
    char audiofilename[32] = {};
    s_audio_start_time_us = rtc_get_epoch_us();
    snprintf(audiofilename, sizeof(audiofilename) - 1, "%lld.bin", s_audio_start_time_us);

    /* Create/open audio file */
    UINT fx_result = FX_ACCESS_ERROR;
    fx_result = fx_file_create(&sdio_disk, audiofilename);
    if ((fx_result != FX_SUCCESS) && (fx_result != FX_ALREADY_CREATED)) {
    	Error_Handler();
    }

    CETI_LOG("Created new audio file \"%s\"", audiofilename);

    fx_result = fx_file_open(&sdio_disk, &audio_file, audiofilename, FX_OPEN_FOR_WRITE);
    if (fx_result != FX_SUCCESS) {
    	Error_Handler();
    }

    fx_result = fx_file_seek(&audio_file, 0);

    // Set "write complete" "callback"
    fx_result = fx_file_write_notify_set(&audio_file, audio_SDWriteComplete);
    if(fx_result != FX_SUCCESS){
        Error_Handler();
    }
}

int log_audio_raw_write(uint8_t *pData, uint32_t size) {    
    // copy data directly to SD card
    UINT fx_result = fx_file_write(&audio_file, pData, size);
    if (fx_result != FX_SUCCESS) {
        //  ToDo: Handle Errors
    }

    // check if new file needs to be created
    uint32_t now_us = rtc_get_epoch_us(); 
    if (now_us - s_audio_start_time_us >= 5*60*1000000) {
        fx_file_close(&audio_file);
        log_audio_create_raw_file();
    }
    return 0;
}
#endif

void log_audio_enable(void) {
    // enable audio acquisition
#if AUDIO_LOG_TYPE == AUDIO_LOG_RAW
    log_audio_create_raw_file();
    acq_audio_set_log_callback(log_audio_raw_write);
#endif
    acq_audio_enable();

    s_log_audio_enabled = 1;
}

void log_audio_disable(void) {
	if (!s_log_audio_enabled) {
		return;
	}

	//stop data acquisition
	acq_audio_disable();

    // ToDo: flush buffered data

	// ToDo: Close file
    fx_file_close(&audio_file);

    s_log_audio_enabled = 0;
}

void log_audio_task(void) {
    // check if audio needs to be written to disk
    acq_audio_task();
}

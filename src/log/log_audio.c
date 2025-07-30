// local files
#include "log_audio.h"
#include "acq/acq_audio.h"

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

extern RTC_HandleTypeDef hrtc;
extern FX_MEDIA sdio_disk;
FX_FILE audio_file = {};

void audio_SDWriteComplete(FX_FILE *file) {
	sd_card_busy = 0;
	s_audio_log_page ^= 1; // advance read head
	// HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
}

static void log_audio_raw_enable(void) {
    /* Create file based on RTC time */
    RTC_DateTypeDef date;   
    RTC_TimeTypeDef time;
    struct tm datetime;
    time_t timestamp;
    char audiofilename[32] = {};

    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    datetime.tm_year = date.Year + 100;
    datetime.tm_mday = date.Date + 1;
    datetime.tm_mon = date.Month + 1;
    datetime.tm_hour = time.Hours;
    datetime.tm_min = time.Minutes;
    datetime.tm_sec = time.Seconds;
    timestamp = mktime(&datetime);
    snprintf(audiofilename, sizeof(audiofilename) - 1, "%lld.bin", timestamp);

    /* Create/open audio file */
    UINT fx_result = FX_ACCESS_ERROR;
    fx_result = fx_file_create(&sdio_disk, audiofilename);
    if ((fx_result != FX_SUCCESS) && (fx_result != FX_ALREADY_CREATED)) {
    	Error_Handler();
    }

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

static void log_audio_raw_update(void) {
	// copy data directly to SD card
    //  UINT fx_result = fx_file_write(&audio_file, s_ret_buffer.half[s_audio_log_page], AUDIO_CIRCULAR_BUFFER_SIZE*RETAIN_BUFFER_SIZE_BLOCKS/2);
    //  if (fx_result != FX_SUCCESS) {
     //ToDo: Handle Errors
    //  }
    /*
    // ToDo: check if new file needs to be created
    // uint32_t now = rtc_get_epoch(); 
    // if (now - audio_start_time >= 5minutes) {
    //     fx_file_close(&audio_file);
    //     snprintf(audiofilename, sizeof(audiofilename) - 1, "%lld.bin", now);
            UINT fx_result = FX_ACCESS_ERROR;
            fx_result = fx_file_create(&sdio_disk, audiofilename);
            if ((fx_result != FX_SUCCESS) && (fx_result != FX_ALREADY_CREATED)) {
    // //ToDo: Handle Errors   
            }

            fx_result = fx_file_open(&sdio_disk, &audio_file, audiofilename, FX_OPEN_FOR_WRITE);
            if (fx_result != FX_SUCCESS) {
    // //ToDo: Handle Errors   
            }

            fx_result = fx_file_seek(&audio_file, 0);

            // Set "write complete" "callback"
            fx_result = fx_file_write_notify_set(&audio_file, audio_SDWriteComplete);
            if(fx_result != FX_SUCCESS){
    // //ToDo: Handle Errors   
            }
            //}
    */
}

void log_audio_enable(void) {
    // enable audio acquisition
    acq_audio_enable();

    if (AUDIO_LOG_TYPE == AUDIO_LOG_RAW) {
        log_audio_raw_enable();
    }

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
    if (AUDIO_LOG_TYPE == AUDIO_LOG_RAW) {
        log_audio_raw_update();
    }
}

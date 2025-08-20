#include "timing.h"

#include <stdint.h>

#include "main.h"

extern RTC_HandleTypeDef hrtc;
extern TIM_HandleTypeDef htim4;


static int timing_has_synced = 0;

time_t s_timer_start_rtc_epoch_us = 0;
uint32_t s_timer_start_timer_count_us = 0;

void rtc_init(void) {
  RTC_DateTypeDef date;
  RTC_TimeTypeDef time;
  struct tm datetime;
  time_t seconds;
  time_t subseconds_us;

  HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
  HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
  datetime.tm_year = date.Year + 100;
  datetime.tm_mday = date.Date;
  datetime.tm_mon = date.Month + 1;
  datetime.tm_hour = time.Hours;
  datetime.tm_min = time.Minutes;
  datetime.tm_sec = time.Seconds;

  seconds = mktime(&datetime);
  subseconds_us = (1000000 * time.SubSeconds) / (time.SecondFraction + 1);

  s_timer_start_rtc_epoch_us = (seconds * 1000000) + subseconds_us;
  HAL_TIM_Base_Start_IT(&uS_htim);
  s_timer_start_timer_count_us = uS_htim.Instance->CNT;
}

time_t rtc_get_epoch_s(void) {
  RTC_DateTypeDef date;
  RTC_TimeTypeDef time;
  struct tm datetime;
  time_t timestamp;

  HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
  HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
  datetime.tm_year = date.Year + 100;
  datetime.tm_mday = date.Date;
  datetime.tm_mon = date.Month + 1;
  datetime.tm_hour = time.Hours;
  datetime.tm_min = time.Minutes;
  datetime.tm_sec = time.Seconds;
  timestamp = mktime(&datetime);
  return timestamp;
}

time_t rtc_get_epoch_ms(void) { return rtc_get_epoch_us() / 1000; }

time_t rtc_get_epoch_us(void) {
  // use the systemclock for better accuracy
  return s_timer_start_rtc_epoch_us + (time_t)timing_get_us_since_on();
}

uint32_t timing_get_us_since_on(void) {
  return (uS_htim.Instance->CNT - s_timer_start_timer_count_us);
}

void timing_task(void) {
  if (timing_has_synced) {
    return;
  }

  // ToDo: check if gps has lock
  // ToDo: get gps time
  // ToDo: set rtc to gps time
  timing_has_synced = 1;
}

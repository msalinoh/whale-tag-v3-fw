
#ifndef UTIL_TIMING_H
#define UTIL_TIMING_H

#include <time.h>

void rtc_init(void);
time_t rtc_get_epoch_s(void);
time_t rtc_get_epoch_ms(void);
time_t rtc_get_epoch_us(void);
uint32_t timing_get_us_since_on(void);

#endif // UTIL_TIMING_H

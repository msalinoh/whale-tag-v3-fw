
#ifndef CETI_CONFIG_H
#define CETI_CONFIG_H

/* AUDIO CONFIG */
#define AUDIO_ENABLED
#ifndef AUDIO_ENABLED
#warning "Audio is currently disabled"
#endif
#define AUDIO_SAMPLE_BITDEPTH (24)
#define AUDIO_SAMPLERATE_SPS  (96000)
#define AUDIO_CH_0_EN         (1)
#define AUDIO_CH_1_EN         (1)
#define AUDIO_CH_2_EN         (1)
#define AUDIO_CH_3_EN         (1)
#define AUDIO_CHANNEL_MASK    (((AUDIO_CH_0_EN) << 0) | ((AUDIO_CH_1_EN) << 0) | ((AUDIO_CH_2_EN) << 0) | (AUDIO_CH_3_EN << 0))

/* BMS CONFIG */
#define BMS_ENABLED
#ifndef BMS_ENABLED
#warning "BMS is currently disabled"
#endif
#define BMS_SAMPLERATE_HZ (1)

/* ECG CONFIG */
// #define ECG_ENABLED
#ifndef ECG_ENABLED
#warning "ECG is currently disabled"
#endif

/* IMU CONFIG */
// #define IMU_ENABLED
#ifndef IMU_ENABLED
#warning "IMU is currently disabled"
#endif

/* GPS CONFIG */
// #define GPS_ENABLED
#ifndef GPS_ENABLED
#warning "GPS is currently disabled"
#endif

/* PRESSURE CONFIG */
#define PRESSURE_ENABLED
#ifndef PRESSURE_ENABLED
#warning "Pressure is currently disabled"
#endif
#define PRESSURE_SAMPLERATE_HZ (1)

/* SATELLITE CONFIG */
// #define SATELLITE_ENABLED
#ifndef SATELLITE_ENABLED
#warning "Satellite communication is currently disabled"
#endif

/* USB CONFIG */
// #define USB_ENABLED
#ifndef USB_ENABLED
#warning "USB is currently disabled"
#endif

#endif // CETI_CONFIG_H

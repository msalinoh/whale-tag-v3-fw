// #ifndef CETI_CONFIG_H
// #define CETI_CONFIG_H

// #include <stdint.h>
// #include "stm32u5xx_hal.h"

// typedef struct 
// {
//     uint32_t id;
//     struct {
//         uint8_t major;
//         uint8_t minor;
//         uint8_t sub;
//         uint8_t __unused;
//     } version;
//     uint32_t deployment_count;
//     uint8_t pad[0x4];
// } NvSettings;

// /* This objects exists in RAM */
// extern NvSettings nv_settings;

// void nv_setting_reload(void);
// HAL_StatusTypeDef nv_settings_write(void);

// #endif // CETI_CONFIG
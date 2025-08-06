// #include "nv_memory.h"

// /* This objects exists in FLASH */
// const NvSettings
// __attribute__((used, section(".nv_settings_sect")))
// nv_settings_flash;

// /* This objects exists in RAM */
// NvSettings nv_settings = nv_settings_flash;

// void nv_setting_reload(void) = {
//     nv_settings = nv_setting_flash;
// }

// HAL_StatusTypeDef nv_settings_write(void) = {
//     HAL_StatusTypeDef result = HAL_FLASH_Unlock();
//     if (result != HAL_OK) {
//         return result;
//     }

//     result = HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD,
//     &nv_settings_flash, &nv_settings); HAL_FLASH_Lock(); return result;
// }
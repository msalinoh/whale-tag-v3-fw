/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32u5xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define AUDIO_V__EN_GPIO_Output_Pin GPIO_PIN_2
#define AUDIO_V__EN_GPIO_Output_GPIO_Port GPIOE
#define Audio_V__EN_GPIO_Output_Pin GPIO_PIN_3
#define Audio_V__EN_GPIO_Output_GPIO_Port GPIOE
#define BMS_I2C3_SCL_Pin GPIO_PIN_0
#define BMS_I2C3_SCL_GPIO_Port GPIOC
#define BMS_I2C3_SDA_Pin GPIO_PIN_1
#define BMS_I2C3_SDA_GPIO_Port GPIOC
#define OPTICAL_SPI2_MISO_Pin GPIO_PIN_2
#define OPTICAL_SPI2_MISO_GPIO_Port GPIOC
#define OPTICAL_SPI2_MOSI_Pin GPIO_PIN_3
#define OPTICAL_SPI2_MOSI_GPIO_Port GPIOC
#define BURNWIRE_EN_GPIO_Output_Pin GPIO_PIN_0
#define BURNWIRE_EN_GPIO_Output_GPIO_Port GPIOA
#define AUDIO_SPI1_SCK_Pin GPIO_PIN_1
#define AUDIO_SPI1_SCK_GPIO_Port GPIOA
#define SATELLITE_USART2_TX_Pin GPIO_PIN_2
#define SATELLITE_USART2_TX_GPIO_Port GPIOA
#define SATELLITE_USART2_RX_Pin GPIO_PIN_3
#define SATELLITE_USART2_RX_GPIO_Port GPIOA
#define AUDIO_SPI_CS_GPIO_Output_Pin GPIO_PIN_4
#define AUDIO_SPI_CS_GPIO_Output_GPIO_Port GPIOA
#define AUDIO_NRST_GPIO_Output_Pin GPIO_PIN_5
#define AUDIO_NRST_GPIO_Output_GPIO_Port GPIOA
#define AUDIO_SPI1_MISO_Pin GPIO_PIN_6
#define AUDIO_SPI1_MISO_GPIO_Port GPIOA
#define AUDIO_SPI1_MOSI_Pin GPIO_PIN_7
#define AUDIO_SPI1_MOSI_GPIO_Port GPIOA
#define SATELLITE_NRST_GPIO_Output_Pin GPIO_PIN_1
#define SATELLITE_NRST_GPIO_Output_GPIO_Port GPIOB
#define FLASHER_EN_GPIO_Output_Pin GPIO_PIN_2
#define FLASHER_EN_GPIO_Output_GPIO_Port GPIOB
#define ECG_LOD_P_GPIO_Input_Pin GPIO_PIN_7
#define ECG_LOD_P_GPIO_Input_GPIO_Port GPIOE
#define ECG_LOD_N_GPIO_Input_Pin GPIO_PIN_8
#define ECG_LOD_N_GPIO_Input_GPIO_Port GPIOE
#define ECG_NRDY_GPIO_Input_Pin GPIO_PIN_9
#define ECG_NRDY_GPIO_Input_GPIO_Port GPIOE
#define ECG_I2C_SCL_Pin GPIO_PIN_10
#define ECG_I2C_SCL_GPIO_Port GPIOB
#define ECG_I2C2_SDA_Pin GPIO_PIN_11
#define ECG_I2C2_SDA_GPIO_Port GPIOB
#define OPTICAL_SPI2_SCK_Pin GPIO_PIN_13
#define OPTICAL_SPI2_SCK_GPIO_Port GPIOB
#define GPS_PPS_RTC_REFIN_Pin GPIO_PIN_15
#define GPS_PPS_RTC_REFIN_GPIO_Port GPIOB
#define IMU_NINT_GPIO_Input_Pin GPIO_PIN_10
#define IMU_NINT_GPIO_Input_GPIO_Port GPIOD
#define IMU_NRESET_GPIO_Output_Pin GPIO_PIN_11
#define IMU_NRESET_GPIO_Output_GPIO_Port GPIOD
#define IMU_I2C4_SCL_Pin GPIO_PIN_12
#define IMU_I2C4_SCL_GPIO_Port GPIOD
#define IMU_I2C4_SDA_Pin GPIO_PIN_13
#define IMU_I2C4_SDA_GPIO_Port GPIOD
#define GPS_USART1_RX_Pin GPIO_PIN_10
#define GPS_USART1_RX_GPIO_Port GPIOA
#define NEOPIXEL_TIM2_CH1_Pin GPIO_PIN_15
#define NEOPIXEL_TIM2_CH1_GPIO_Port GPIOA
#define GPS_USART1_TX_Pin GPIO_PIN_6
#define GPS_USART1_TX_GPIO_Port GPIOB
#define SENSOR_I2C_SDA_Pin GPIO_PIN_7
#define SENSOR_I2C_SDA_GPIO_Port GPIOB
#define SENSOR_I2C1_SCL_Pin GPIO_PIN_8
#define SENSOR_I2C1_SCL_GPIO_Port GPIOB
#define KELLER_DRDY_GPIO_Input_Pin GPIO_PIN_9
#define KELLER_DRDY_GPIO_Input_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

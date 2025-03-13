/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins
     PC14-OSC32_IN (PC14)   ------> RCC_OSC32_IN
     PH0-OSC_IN (PH0)   ------> RCC_OSC_IN
     PA8   ------> RCC_MCO
     PA13 (JTMS/SWDIO)   ------> DEBUG_JTMS-SWDIO
     PA14 (JTCK/SWCLK)   ------> DEBUG_JTCK-SWCLK
     PB3 (JTDO/TRACESWO)   ------> DEBUG_JTDO-SWO
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, AUDIO_V__EN_GPIO_Output_Pin|Audio_V__EN_GPIO_Output_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, BURNWIRE_EN_GPIO_Output_Pin|AUDIO_SPI_CS_GPIO_Output_Pin|AUDIO_NRST_GPIO_Output_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SATELLITE_NRST_GPIO_Output_Pin|FLASHER_EN_GPIO_Output_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(IMU_NRESET_GPIO_Output_GPIO_Port, IMU_NRESET_GPIO_Output_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : AUDIO_V__EN_GPIO_Output_Pin Audio_V__EN_GPIO_Output_Pin */
  GPIO_InitStruct.Pin = AUDIO_V__EN_GPIO_Output_Pin|Audio_V__EN_GPIO_Output_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PC13 PC15 PC6 PC7 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_15|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PH1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pins : BURNWIRE_EN_GPIO_Output_Pin AUDIO_SPI_CS_GPIO_Output_Pin AUDIO_NRST_GPIO_Output_Pin */
  GPIO_InitStruct.Pin = BURNWIRE_EN_GPIO_Output_Pin|AUDIO_SPI_CS_GPIO_Output_Pin|AUDIO_NRST_GPIO_Output_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB14 PB4 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_14|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : SATELLITE_NRST_GPIO_Output_Pin FLASHER_EN_GPIO_Output_Pin */
  GPIO_InitStruct.Pin = SATELLITE_NRST_GPIO_Output_Pin|FLASHER_EN_GPIO_Output_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : ECG_LOD_P_GPIO_Input_Pin ECG_LOD_N_GPIO_Input_Pin ECG_NRDY_GPIO_Input_Pin */
  GPIO_InitStruct.Pin = ECG_LOD_P_GPIO_Input_Pin|ECG_LOD_N_GPIO_Input_Pin|ECG_NRDY_GPIO_Input_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PE10 PE11 PE12 PE13
                           PE14 PE15 PE0 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13
                          |GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PD8 PD9 PD14 PD15
                           PD0 PD1 PD3 PD4
                           PD5 PD6 PD7 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_14|GPIO_PIN_15
                          |GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_3|GPIO_PIN_4
                          |GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : IMU_NINT_GPIO_Input_Pin */
  GPIO_InitStruct.Pin = IMU_NINT_GPIO_Input_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(IMU_NINT_GPIO_Input_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : IMU_NRESET_GPIO_Output_Pin */
  GPIO_InitStruct.Pin = IMU_NRESET_GPIO_Output_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(IMU_NRESET_GPIO_Output_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : KELLER_DRDY_GPIO_Input_Pin */
  GPIO_InitStruct.Pin = KELLER_DRDY_GPIO_Input_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(KELLER_DRDY_GPIO_Input_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */

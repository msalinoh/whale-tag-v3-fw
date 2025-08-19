/*****************************************************************************
 *   @file      imu/acq_imu.h
 *   @brief     IMU sample acquisition and buffering code
 *   @project   Project CETI
 *   @copyright Harvard University Wood Lab
 *   @authors   Michael Salino-Hugg, [TODO: Add other contributors here]
 *****************************************************************************/
#ifndef CETI_ACQ_IMU_H
#define CETI_ACQ_IMU_H

void acq_imu_init(void);
void acq_imu_start(void);
void acq_imu_task(void);
void acq_imu_EXTI_Callback(void);

#endif // CETI_ACQ_IMU_H
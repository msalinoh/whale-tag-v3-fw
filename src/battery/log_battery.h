/*****************************************************************************
 *   @file      battery/log_battery.h
 *   @brief     code for saving acquired battery data to disk
 *   @project   Project CETI
 *   @copyright Harvard University Wood Lab
 *   @authors   Michael Salino-Hugg, [TODO: Add other contributors here]
 *****************************************************************************/
#ifndef CETI_LOG_BATTERY_H
#define CETI_LOG_BATTERY_H

void log_battery_enable(void);
void log_battery_task(void);

#endif // CETI_LOG_BATTERY_H

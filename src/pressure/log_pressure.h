/*****************************************************************************
 *   @file      log_pressure.h
 *   @brief     pressure processing and storing code.
 *   @project   Project CETI
 *   @copyright Harvard University Wood Lab
 *   @authors   Michael Salino-Hugg, [TODO: Add other contributors here]
 *****************************************************************************/
#ifndef CETI_LOG_PRESSURE_H
#define CETI_LOG_PRESSURE_H

#include "acq_pressure.h"

void log_pressure_enable(void);
void log_pressure_disable(void);

void log_pressure_task(void);

#endif // CETI_LOG_PRESSURE_H

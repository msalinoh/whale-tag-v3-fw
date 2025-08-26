/*****************************************************************************
 *   @file      gps/gps.h
 *   @brief     
 *   @project   Project CETI
 *   @copyright Harvard University Wood Lab
 *   @authors   Michael Salino-Hugg, [TODO: Add other contributors here]
 *****************************************************************************/
#ifndef CETI_GPS_H
#define CETI_GPS_H

#include "minmea.h"

void gps_init(void);
void gps_start(void);
void gps_task(void);
int gps_get_latest_timestamp(struct minmea_date *pDate, struct minmea_time *pTime);
int gps_get_latest_position(struct minmea_float *pLatitude, struct minmea_float *pLongitude);
#endif // CETI_GPS_H

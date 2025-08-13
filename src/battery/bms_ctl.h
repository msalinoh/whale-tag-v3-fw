/*****************************************************************************
 *   @file      battery/bms_ctl.h
 *   @brief     High level runtime BMS control
 *   @project   Project CETI
 *   @copyright Harvard University Wood Lab
 *   @authors   Michael Salino-Hugg, [TODO: Add other contributors here]
 *****************************************************************************/
#ifndef CETI_BMS_CTL_H
#define CETI_BMS_CTL_H

int bms_ctl_verify(void);
int bms_ctl_program_nonvolatile_memory(void);
int bms_ctl_temporary_overwrite_nv_values(void);

#endif // CETI_BMS_CTL_H
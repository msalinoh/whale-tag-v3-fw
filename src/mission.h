
//-----------------------------------------------------------------------------
// Project: CETI Tag Electronics
// Copyright: Harvard University Wood Lab
// Contributors: Michael Salino-Hugg, [TODO: Add other contributors here]
//-----------------------------------------------------------------------------
#ifndef CETI_WHALE_TAG_MISSON_H
#define CETI_WHALE_TAG_MISSON_H

typedef enum {
	MISSION_STATE_PREDEPLOY,
	MISSION_STATE_OFFLOAD,
	MISSION_STATE_SURFACE,
	MISSION_STATE_DIVE,
	MISSION_STATE_BURN,
	MISSION_STATE_RETRIEVE,
	MISSION_STATE_SHUTDOWN,
	MISSION_STATE_DEBUG,
	MISSION_STATE_ERROR
} MissionState;


void mission_task(void);

#endif // CETI_WHALE_TAG_MISSION_H

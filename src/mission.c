//-----------------------------------------------------------------------------
// Project: CETI Tag Electronics
// Copyright: Harvard University Wood Lab
// Contributors: Michael Salino-Hugg, [TODO: Add other contributors here]
//-----------------------------------------------------------------------------

#include "mission.h"

#include "audio/log_audio.h"
#include "battery/log_battery.h"
#include "main.h"

static MissionState s_state = MISSION_STATE_ERROR;

void mission_set_state(MissionState next_state) {
	MissionState previous_state = s_state;
	if (next_state == previous_state) {
		return;
	}
	
	// disable resources used in current state but not next state
	switch (next_state) {
		case MISSION_STATE_SURFACE:
//			log_pressure_init();
			break;
		default:
			break;
	}

	// enable resources not used in the current state, but used in the next state

	//update state
	s_state = next_state;
}


/**
 * @brief updates the current system state based on latest system interaction 
 */
void mission_task(void) {
	switch (s_state) {
		case MISSION_STATE_SURFACE:
			/* sleep until an interrupt says there's something to do */ 
			// ToDo: disable unused clocks
			HAL_SuspendTick();
			__disable_irq();
			HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
			/*
			* code to execute prior to jumping to IRQ here
			*/
			__enable_irq();
			// ToDo: reenable clocks
			HAL_ResumeTick();

			/* tasks to execute on wake */

			while(1) {
				/* high priority - always check these*/
				log_audio_task();
				log_battery_task();

				/* normal priority - sequence these, but don't block high priority*/
				// ToDo: log depth
				// ToDo: log GPS
				// ToDo: log IMU
				// ToDo: log ecg
				// ToDo: log syslog task
				break; // all tasks serviced exit loop;
			}
			break;

		case MISSION_STATE_DIVE:
			/* sleep until an interrupt says there's something to do */ 
			// ToDo: disable unused clocks
			HAL_SuspendTick();
			__disable_irq();
			HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
			/*
			* code to execute prior to jumping to IRQ here
			*/
			__enable_irq();
			// ToDo: reenable clocks
			HAL_ResumeTick();

			/* tasks to execute on wake */

			while(1) {
				/* high priority - always check these*/
				log_audio_task();
				log_battery_task();

				break; // all tasks serviced exit loop;
			}			break;

		case MISSION_STATE_BURN:
			log_audio_task();
			log_battery_task();

			break;

		case MISSION_STATE_RETRIEVE:
			// disable audio recordings
			log_battery_task();

			break;

		case MISSION_STATE_SHUTDOWN:
			break;

		case MISSION_STATE_DEBUG:
			break;

		case MISSION_STATE_ERROR:
			break;
	}
	
}

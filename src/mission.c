//-----------------------------------------------------------------------------
// Project: CETI Tag Electronics
// Copyright: Harvard University Wood Lab
// Contributors: Michael Salino-Hugg, [TODO: Add other contributors here]
//-----------------------------------------------------------------------------

#include "mission.h"

#include "audio/log_audio.h"
#include "battery/log_battery.h"
#include "pressure/log_pressure.h"
#include "main.h"

static MissionState s_state = MISSION_STATE_ERROR;
#define MISSION_DIVE_THRESHOLD_BAR (5.0)
#define MISSION_SURFACE_THRESHOLD_BAR (1.0)
#define MISSION_BURN_THRESHOLD_CELL_V (3.3)
#define MISSION_CRITICAL_THRESHOLD_CELL_V (3.2)

typedef void (*MissionTask)(void); 
MissionTask mission_surface_tasks[] = {
	log_battery_task,
	log_pressure_task,
	// log_gps_task,
	// log_imu_task,
	// log_ecg_task,
	// log_syslog_task,
};

MissionTask mission_dive_tasks[] = {
	log_battery_task,
	log_pressure_task,
	// log_gps_task,
	// log_imu_task,
	// log_ecg_task,
	// log_syslog_task,
};

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
		case MISSION_STATE_SURFACE: {
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
			for (int i = 0; i < sizeof(mission_surface_tasks)/sizeof(mission_surface_tasks[0]); i++){
				log_audio_task(); // always check if audio needs to be logged
				mission_surface_tasks[i](); // perform an additional task
			}

			/* update state machine */
			CetiBatterySample battery_sample;
			acq_battery_peak_latest_sample(&battery_sample);
			if( (battery_sample.cell_voltage_v[0] <= MISSION_BURN_THRESHOLD_CELL_V) 
				|| (battery_sample.cell_voltage_v[1] <= MISSION_BURN_THRESHOLD_CELL_V) 
			){
				mission_set_state(MISSION_STATE_BURN);
			}

			CetiPressureSample pressure_sample;
			acq_pressure_peak_latest_sample(&pressure_sample);
			if(pressure_sample.data.pressure >= KELLER4LD_PRESSURE_BAR_TO_RAW(MISSION_DIVE_THRESHOLD_BAR)) {
				mission_set_state(MISSION_STATE_DIVE);
				break;
			}
		}
		break;

		case MISSION_STATE_DIVE: {
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
			for (int i = 0; i < sizeof(mission_dive_tasks)/sizeof(mission_dive_tasks[0]); i++){
				log_audio_task(); // always check if audio needs to be logged
				mission_dive_tasks[i](); // perform an additional task
			}

			/* update state machine */
			CetiBatterySample battery_sample;
			acq_battery_peak_latest_sample(&battery_sample);
			if( (battery_sample.cell_voltage_v[0] <= MISSION_BURN_THRESHOLD_CELL_V) 
				|| (battery_sample.cell_voltage_v[1] <= MISSION_BURN_THRESHOLD_CELL_V) 
			){
				mission_set_state(MISSION_STATE_BURN);
			}

			CetiPressureSample pressure_sample;
			acq_pressure_peak_latest_sample(&pressure_sample);
			if(pressure_sample.data.pressure >= KELLER4LD_PRESSURE_BAR_TO_RAW(MISSION_DIVE_THRESHOLD_BAR)) {
				mission_set_state(MISSION_STATE_DIVE);
				break;
			}
		}
		break;

		case MISSION_STATE_BURN: {
			log_audio_task();
			log_battery_task();
		}
		break;

		case MISSION_STATE_RETRIEVE: {
			// disable audio recordings
			log_battery_task();
		}
		break;

		case MISSION_STATE_SHUTDOWN:
			break;

		case MISSION_STATE_DEBUG:
			break;

		case MISSION_STATE_ERROR:
			break;
	}
	
}

#include "main.h"

typedef enum {
    LOG_BATTERY_FORMAT_BIN,
    LOG_BATTERY_FORMAT_CSV,
} LogBatteryFormat;

#define LOG_BATTERY_FORMAT LOG_BATTERY_FORMAT_CSV


TIM_HandleTypeDef htim2;


#if LOG_BATTERY_FORMAT ==  LOG_BATTERY_FORMAT_BIN
void log_battery_write_bin(void) {

}
#elif LOG_BATTERY_FORMAT ==  LOG_BATTERY_FORMAT_CSV
void log_battery_write_csv(void) {
    // convert battery data into csv lines via write scratch buffer
}
#endif



void log_battery_enable(void) {
    // ToDo: create file for battery data
    // ToDo: start battery acquisition
}
#include "main.h"

typedef enum {
    ACQ_GPS_PROTOCOL_UART,
    ACQ_GPS_PROTOCOL_I2C
} GpsProtocol;

#define ACQ_GPS_PROTOCOL ACQ_GPS_PROTOCOL_UART

void acq_gps_enable(void) {
#if ACQ_GPS_PROTOCOL == ACQ_GPS_PROTOCOL_I2C
    // ToDo: setup I2C comms
#else // ACQ_GPS_PROTOCOL_UART
    // ToDo: setup UART comms
#endif
    // start GPS
    HAL_GPIO_WritePin(GPS_PWR_EN_GPIO_Output_GPIO_Port, GPS_PWR_EN_GPIO_Output_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPS_NRST_GPIO_Output_GPIO_Port, GPS_NRST_GPIO_Output_Pin, GPIO_PIN_RESET);


}

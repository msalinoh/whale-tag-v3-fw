#include "main.h"

#include "usart.h"

#define NMEA_MAX_SIZE (82)

typedef enum {
    ACQ_GPS_PROTOCOL_UART,
    ACQ_GPS_PROTOCOL_I2C
} GpsProtocol;

#define ACQ_GPS_PROTOCOL ACQ_GPS_PROTOCOL_UART

extern UART_HandleTypeDef GPS_huart;

uint8_t max10s_rx_buffer[2][16*NMEA_MAX_SIZE];
void acq_gps_RxCpltCallback(UART_HandleTypeDef *huart) {
    __NOP();
}

void acq_gps_enable(void) {
    /* setup UART*/
    MX_USART1_UART_Init();
	HAL_UART_RegisterCallback(&GPS_huart, HAL_UART_RX_COMPLETE_CB_ID, acq_gps_RxCpltCallback);
	HAL_UART_RegisterCallback(&GPS_huart, HAL_UART_RX_HALFCOMPLETE_CB_ID, acq_gps_RxCpltCallback);
	HAL_UART_Receive_DMA(&GPS_huart, &max10s_rx_buffer[0][0], sizeof(max10s_rx_buffer));

    // start GPS
    HAL_GPIO_WritePin(GPS_PWR_EN_GPIO_Output_GPIO_Port, GPS_PWR_EN_GPIO_Output_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPS_NRST_GPIO_Output_GPIO_Port, GPS_NRST_GPIO_Output_Pin, GPIO_PIN_RESET);
}

#include "main.h"

#include "usart.h"

#include <string.h>

const char *ping_cmd = "AT+PING=?";

int satellite_ping(void) {
    uint8_t response[0x10] = {0};
    HAL_UART_Transmit(&huart2, (uint8_t *)ping_cmd, strlen(ping_cmd), 100);
    
    uint8_t offset = 0xff;
    do {
        offset += 1;
        HAL_UART_Receive(&huart2, &response[offset], 1, 5);
    } while((response[offset] != '\n') && (offset < 0x10));

    return response[8] = '1';
}

int satellite_tx_message(uint8_t *message, size_t message_len) {
    return 0;
}

void satellite_init(void) { 

    /* configure gpios */
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    HAL_GPIO_WritePin(GPIOE, SAT_PWR_EN_GPIO_Output_Pin | SAT_PM_2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, SAT_NRST_GPIO_Output_Pin | SAT_PM_1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, SAT_RF_NRST_Pin, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = SAT_PWR_EN_GPIO_Output_Pin | SAT_PM_2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = SAT_NRST_GPIO_Output_Pin|SAT_PM_1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = SAT_RF_NRST_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = SAT_RF_BUSY_GPIO_Input_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* configure uart*/
    MX_USART2_UART_Init();

    /* Reset module */
    HAL_GPIO_WritePin(SAT_PWR_EN_GPIO_Output_GPIO_Port, SAT_PWR_EN_GPIO_Output_Pin, GPIO_PIN_SET);
    HAL_Delay(500);
    int established_comms = satellite_ping();
    
    if (!established_comms) {
        // error
    }

}

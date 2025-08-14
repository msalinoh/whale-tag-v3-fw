/*****************************************************************************
 *   @file      imu/acq_imu.c
 *   @brief     IMU sample acquisition and buffering code
 *   @project   Project CETI
 *   @copyright Harvard University Wood Lab
 *   @authors   Michael Salino-Hugg, [TODO: Add other contributors here]
 *****************************************************************************/
#include "sh2.h"
#include "sh2_err.h"

#include "main.h"
#include "spi.h"

#include "timing.h"

#include <string.h>

#define BNO08X_HDR_LEN (4)

static uint8_t txBuf[SH2_HAL_MAX_TRANSFER_OUT];
static uint8_t rxBuf[SH2_HAL_MAX_TRANSFER_IN];

#define imu_ncs(value) HAL_GPIO_WritePin(IMU_NCS_GPIO_Output_GPIO_Port, IMU_NCS_GPIO_Output_Pin, (value))

static void acq_imu_spi_close(sh2_Hal_t *self);
static int acq_imu_spi_open(sh2_Hal_t *self);
static int acq_imu_spi_hal_read(sh2_Hal_t *self, uint8_t *pBuffer, unsigned len, uint32_t *t);
static int acq_imu_spi_hal_write(sh2_Hal_t *self, uint8_t *pBuffer, unsigned len);

typedef enum {
    SPI_INIT,
    SPI_DUMMY,
    SPI_IDLE,
    SPI_RD_HDR,
    SPI_RD_BODY,
    SPI_WRITE
} SpiState;

static int s_spi_is_open = 0;
static volatile time_t s_rx_timestamp_us;
static volatile uint32_t s_rx_buf_len;
static volatile uint32_t s_tx_buf_len;
static volatile int s_rx_ready;
static volatile SpiState s_spi_state = SPI_INIT;
static sh2_Hal_t bno08x = {
        .close = acq_imu_spi_close,
        .open = acq_imu_spi_open,
        .read = acq_imu_spi_hal_read,
        .write = acq_imu_spi_hal_write,
};

static void acq_imu_start_spi_transfer(void) {
    if (!((s_spi_state == SPI_IDLE) && (s_rx_buf_len == 0) && s_rx_ready)) {
        return; // spi is busy
    }
    s_rx_ready = 0;
    imu_ncs(GPIO_PIN_RESET); 
    if (s_tx_buf_len > 0) {
        s_spi_state = SPI_WRITE;
        HAL_SPI_TransmitReceive_IT(&IMU_hspi, txBuf, rxBuf, s_tx_buf_len);
        // Deassert Wake
        // ps0_waken(true);
    } else {
        s_spi_state = SPI_RD_HDR;
        HAL_SPI_Receive_IT(&IMU_hspi, rxBuf, BNO08X_HDR_LEN);
    }

}

void acq_imu_spi_complete_callback(void) {
    // Get length of payload avaiable
    uint16_t rxLen = (rxBuf[1] & ~0x80) << 8 | rxBuf[0];

    rxLen = (rxLen > sizeof(rxBuf)) ? sizeof(rxBuf) : rxLen;
    switch (s_spi_state) {
        case SPI_DUMMY:
            s_spi_state = SPI_IDLE;
            break;

        case SPI_RD_HDR:
            if (rxLen > BNO08X_HDR_LEN) {
                s_spi_state = SPI_RD_BODY;
                HAL_SPI_Receive_IT(&IMU_hspi, rxBuf, rxLen - BNO08X_HDR_LEN);
                break;
            } 
            __attribute__ ((fallthrough));
        case SPI_RD_BODY:
            imu_ncs(GPIO_PIN_SET);
            s_rx_buf_len = 0;
            s_spi_state = SPI_IDLE;
            acq_imu_start_spi_transfer();
            break;           
        
        case SPI_WRITE:
            imu_ncs(GPIO_PIN_SET);
            s_rx_buf_len = (s_tx_buf_len < rxLen) ? s_tx_buf_len : rxLen;
            s_tx_buf_len = 0;
            s_spi_state = SPI_IDLE;
            acq_imu_start_spi_transfer();
            break;

        default:
            break;
    }

}

void acq_imu_EXTI_Callback(void) {
    s_rx_timestamp_us = 
    // ToDo: store timeStamp
    // inReset = 0;
    s_rx_ready = 1;
    acq_imu_start_spi_transfer();
}

void acq_imu_disable_interrupts(void) {
    // HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
    HAL_NVIC_DisableIRQ(SPI1_IRQn);
}

void acq_imu_enable_interrupts(void) {
    // HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
    HAL_NVIC_EnableIRQ(SPI1_IRQn);
}

static int acq_imu_spi_open(sh2_Hal_t *self) {
    MX_SPI1_Init();
    if (s_spi_is_open) {
        return SH2_ERR; // can't open another instance
    }
    s_spi_is_open = 1;
    

    // initialize spi hardware
    imu_ncs(GPIO_PIN_SET);
    return SH2_OK;
}

static void acq_imu_spi_close(sh2_Hal_t *self) {
    HAL_SPI_DeInit(&IMU_hspi);
    s_spi_is_open = 0;
    return;
}

static int acq_imu_spi_hal_read(sh2_Hal_t *self, uint8_t *pBuffer, unsigned len, uint32_t *t) {
    int retval = 0;

    // If there is received data available...
    if (s_rx_buf_len == 0) {
        return 0;
    }
    
    if (len < s_rx_buf_len) {
        // Clear rxBuf so we can receive again
        s_rx_buf_len = 0;
        // Now that rxBuf is empty, activate SPI processing to send any
        // potential write that was blocked.
        // disableInts();
        acq_imu_start_spi_transfer();
        // enableInts();
        return SH2_ERR_BAD_PARAM;
    }


    // And if the data will fit in this buffer...
    // Copy data to the client buffer
    memcpy(pBuffer, rxBuf, s_rx_buf_len);
    retval = s_rx_buf_len;

    // Set timestamp of that data
    *t = s_rx_timestamp_us;

    // Clear rxBuf so we can receive again
    s_rx_buf_len = 0;

    // Now that rxBuf is empty, activate SPI processing to send any
    // potential write that was blocked.
    // disableInts();
    acq_imu_start_spi_transfer();
    // enableInts();
    

    return retval;
}

static int acq_imu_spi_hal_write(sh2_Hal_t *self, uint8_t *pBuffer, unsigned len) {
    int retval = SH2_OK;

    // Validate parameters
    if ((self == 0) || (len > sizeof(txBuf)) ||
        ((len > 0) && (pBuffer == 0)))
    {
        return SH2_ERR_BAD_PARAM;
    }

    // If tx buffer is not empty, return 0
    if (s_tx_buf_len != 0) {
        return 0;
    }
    
    // Copy data to tx buffer
    memcpy(txBuf, pBuffer, len);
    s_tx_buf_len = len;
    retval = len;

    // // disable SH2 interrupts for a moment
    // disableInts();

    // // Assert Wake
    // ps0_waken(false);

    // // re-enable SH2 interrupts.
    // enableInts();

    return retval;
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef * hspi) {
    acq_imu_spi_complete_callback();
}

void acq_imu_init(void) {
    sh2_open(&bno08x, NULL, NULL);
}
#include "max17320.h"

#include "stm32u5xx_hal.h"

#define BMS_I2C_DEV_ADDR_UPPER (0x0b) // For internal memory range 180h-1FFh
#define BMS_I2C_DEV_ADDR_LOWER (0x36) // For internal memory range 000h-0FFh

#if HW_VERSION == HW_VERSION_3_1
    #define BMS_hi2c hi2c3
#endif

typedef struct {
    I2C_HandleTypeDef *hi2c;
} Max17320Description;

extern I2C_HandleTypeDef BMS_hi2c;
static Max17320Description self = {
    .hi2c = &BMS_hi2c,
};

static inline double __current_mA_from_raw(uint16_t raw, double r_sense_mOhm) {
    double current_uv = ((double)((int16_t)raw)) * CURRENT_LSB_uV;
    return current_uv / r_sense_mOhm;
}

static inline double __raw_to_capacity_mAh(uint16_t raw, double r_sense_mOhm) {
    return ((double)raw) * 0.005 / r_sense_mOhm;
}

static inline double __raw_to_percentage(uint16_t raw) {
    return ((double)raw) / 256.0;
}

static inline double __raw_to_voltage_v(uint16_t raw) {
    return ((double)raw) * 0.000078125;
}

static inline double __raw_to_current_mA(uint16_t raw, double r_sense_mOhm) {
    return ((double)((int16_t)raw)) * 0.15625 / r_sense_mOhm;
}

static inline float __raw_to_temperature_c(uint16_t raw) {
    return ((float)((int16_t)raw)) / 256.0;
}

static inline double __raw_to_time_s(uint16_t raw) {
    return ((double)raw) * 5.625;
}

/**
 * @brief Read register at specified memory address 
 * 
 * @param memory register memory address
 * @param storage destination pointer
 * @return HAL_StatusTypedef
 */
HAL_StatusTypeDef max17320_read(uint16_t memory, uint16_t *storage) {
    uint16_t addr = BMS_I2C_DEV_ADDR_LOWER;
    if (memory > 0xFF) {
        memory = memory & 0xFF;
        addr = BMS_I2C_DEV_ADDR_UPPER;
    }

    return HAL_I2C_Mem_Read(self.hi2c, (addr << 1), memory, I2C_MEMADD_SIZE_8BIT, (uint8_t *)storage, sizeof(uint16_t), 1000);
}


/**
 * @brief Write to register at specified memory address  
 * 
 * @param memory register memory address
 * @param data 
 * @return HAL_StatusTypedef 
 */
HAL_StatusTypeDef max17320_write(uint16_t memory, uint16_t data) {
    uint16_t addr = BMS_I2C_DEV_ADDR_LOWER;
    if (memory > 0xFF) {
        memory = memory & 0xFF;
        addr = BMS_I2C_DEV_ADDR_UPPER;
    }
    return HAL_I2C_Mem_Write(self.hi2c, (addr << 1), memory, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&data, sizeof(uint16_t), 1000);
}

WTResult max17320_clear_write_protection(void) {
    uint16_t read = 0;

    max17320_write(MAX17320_REG_COMM_STAT, CLEAR_WRITE_PROT);
    HAL_Delay(TRECALL_US/1000);
    max17320_write(MAX17320_REG_COMM_STAT, CLEAR_WRITE_PROT);
    HAL_Delay(TRECALL_US/1000);
    max17320_read(MAX17320_REG_COMM_STAT, &read);

    if (read != CLEARED_WRITE_PROT && read != CLEAR_WRITE_PROT) {
        return -1;
        // return WT_RESULT(WT_DEV_BMS, WT_ERR_BMS_WRITE_PROT_DISABLE_FAIL);
    }
    return WT_OK;
}

#define usleep(time_us) HAL_Delay(time_us/1000)
#define WT_TRY(body) {body;}

WTResult max17320_swap_shadow_ram(void) {
    // Clear CommStat.NVError bit
    WT_TRY(max17320_write(MAX17320_REG_COMM_STAT, CLEAR_WRITE_PROT));

    // Initiate a block copy
    WT_TRY(max17320_write(MAX17320_REG_COMMAND, INITIATE_BLOCK_COPY));
    // TODO: find right value
    usleep(TRECALL_US);
    // wait for block copy to complete
    uint16_t read = 0;
    do {
        WT_TRY(max17320_read(MAX17320_REG_COMM_STAT, &read));
        usleep(TRECALL_US);
        // ToDo: add timeout
    } while (read & 0x0004);

    WT_TRY(max17320_reset());
    return WT_OK;
}

WTResult max17320_gauge_reset(void) {
    WT_TRY(max17320_clear_write_protection());
    // Reset firmware
    WT_TRY(max17320_write(MAX17320_REG_CONFIG2, MAX17320_RESET_FW));
    uint16_t read = 0;
    do {
        WT_TRY(max17320_read(MAX17320_REG_CONFIG2, &read));
        usleep(TRECALL_US);
    } while ((read & 0x8000)); // Wait for POR_CMD bit to clear
    return WT_OK;
}

WTResult max17320_reset(void) {
    // Performs full reset
    WT_TRY(max17320_clear_write_protection());
    WT_TRY(max17320_write(MAX17320_REG_COMMAND, MAX17320_RESET));
    usleep(10000);
    WT_TRY(max17320_gauge_reset());
    return WT_OK;
}

int max17320_get_cell_temperature_raw(int cell_index, uint16_t *tCells) {
    if (cell_index >= MAX17320_CELL_COUNT) {
        return -1;
    }
    return max17320_read(0x13A - cell_index, tCells);
}

int max17320_get_cell_temperature_c(int cell_index, float *tCells_c) {
    uint16_t raw;
    int status = max17320_get_cell_temperature_raw(cell_index,&raw);
    if( status != 0) {
        return status;
    }

    if (tCells_c != NULL) {
        *tCells_c = __raw_to_temperature_c(raw);
    }
    return 0;
}

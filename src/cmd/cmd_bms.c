#include "device/max17320.h"
#include "log/log_syslog.h"
#include "util/error.h"

typedef struct {
    char *name;
    uint16_t addr;
    uint16_t value;
} NvExpected;

const NvExpected g_nv_expected[] = {
    {.name = "NRSENSE", .addr = 0x1cf, .value = 0x03e8},
     {.name = "NDESIGNCAP", .addr = 0x1b3, .value = 0x2710},
//    {.name = "NDESIGNCAP", .addr = 0x1b3, .value = 0x1F40},
    {.name = "NPACKCFG", .addr = 0x1b5, .value = 0xc208},
    {.name = "NNVCFG0", .addr = 0x1b8, .value = 0x0830},
    {.name = "NNVCFG1", .addr = 0x1b9, .value = 0x2100},
    {.name = "NNVCFG2", .addr = 0x1ba, .value = 0x822d},
    {.name = "NUVPRTTH", .addr = 0x1d0, .value = 0xa002},
    {.name = "NTPRTTH1", .addr = 0x1d1, .value = 0x280a},
    {.name = "NIPRTTH1", .addr = 0x1d3, .value = 0x32ce},
    {.name = "NBALTH", .addr = 0x1d4, .value = 0x0ca0},
    {.name = "NPROTMISCTH", .addr = 0x1d6, .value = 0x0813},
    {.name = "NPROTCFG", .addr = 0x1d7, .value = 0x0c08},
    {.name = "NJEITAV", .addr = 0x1d9, .value = 0xec00},
    {.name = "NOVPRTTH", .addr = 0x1da, .value = 0xb3a0},
    {.name = "NDELAYCFG", .addr = 0x1dc, .value = 0x0035},
    {.name = "NODSCCFG", .addr = 0x1de, .value = 0x4058},
    {.name = "NCONFIG", .addr = 0x1b0, .value = 0x0290},
    {.name = "NTHERMCFG", .addr = 0x1ca, .value = 0x71be},
    {.name = "NVEMPTY", .addr = 0x19e, .value = 0x9659},
    {.name = "NFULLSOCTHR", .addr = 0x1c6, .value = 0x5005},
    {.name = NULL},

};

/**
 * @brief Read BMS nonvolatile memory and verifies values are the expected value.
 *
 * @return int true if match, else false
 */
int cmd_bms_verify(void) {
    // char err_str[512];
    int incorrect = 0;
    CETI_LOG("Nonvoltile RAM Settings:"); // echo it
    for (int i = 0; g_nv_expected[i].name != NULL; i++) {
        uint16_t actual;

        // hardware access register
        WTResult result = max17320_read(g_nv_expected[i].addr, &actual);
        if (result != WT_OK) {
            // CETI_ERR("BMS device read error: %s\n", wt_strerror_r(result, err_str, sizeof(err_str)));
            return 0;
        }

        // assertions
        if (actual != g_nv_expected[i].value) {
           CETI_WARN("%-12s: 0x%04x != 0x%04x !!!!", g_nv_expected[i].name, actual, g_nv_expected[i].value);
            incorrect++;
        } else {
           CETI_LOG("%-12s: 0x%04x  OK!\n", g_nv_expected[i].name, actual);
        }
    }

    if (incorrect != 0) {
       CETI_WARN("%d values did not match expected value", incorrect);
        return 0;
    }
    return 1;
}

int cmd_bms_program_nonvolatile_memory(void) {
    WTResult hw_result = max17320_clear_write_protection();
    if (hw_result != 0) {
    	return -1;
    }
    //write shadowram
    for (int i = 0; g_nv_expected[i].name != NULL; i++) {
        // hardware access register
        hw_result |= max17320_write(g_nv_expected[i].addr, g_nv_expected[i].value);
        if (hw_result != WT_OK) {
            // CETI_ERR("BMS device read error: %s\n", wt_strerror_r(result, err_str, sizeof(err_str)));
            return hw_result;
        }
    }
    hw_result = max17320_swap_shadow_ram();
    return 0;
}


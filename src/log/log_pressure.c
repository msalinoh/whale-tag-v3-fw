#include <stdint.h>

typedef struct {
    // timestamp
    uint32_t status;
    uint16_t pressure_raw;
    uint16_t temperature_raw; 
} CetiPressureSampleRaw;
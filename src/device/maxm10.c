#include <stdint.h>

typedef struct {
    uint8_t a;
    uint8_t b;
} UbxChecksum;

typedef struct {
    uint8_t class;
    uint8_t id;
    uint16_t length;
    struct{
        uint8_t *ptr;
        uint16_t length;
    } payload;
    UbxChecksum checksum;
}UbxMessage;

UbxChecksum ubx_calculate_checksum(const uint8_t *message, uint16_t msg_length) {
    UbxChecksum cs = {.a = 0, .b = 0};
    for(int i = 0; i < msg_length; i++) {
        cs.a += message[i];
        cs.b += cs.a;
    }
    return cs;
}

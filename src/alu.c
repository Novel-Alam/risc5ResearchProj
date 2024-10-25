#include "alu.h"

decoder_to_execute aluOut;

uint32_t alu_add(uint32_t a, uint32_t b) {
    return a + b;
}

uint32_t alu_sub(uint32_t a, uint32_t b) {
    return a - b;
}
#include "alu.h"

decoder_to_execute aluOut;

uint32_t alu_add(uint32_t a, uint32_t b) {
    return a+b;
}

void alu_add_wrapper(uint32_t a, uint32_t b, uint32_t *result, uint32_t *flags) {
    *result = alu_add(a, b);

    *flags = 0;
    if (*result == 0) *flags |= ZERO_FLAG;
    if (*result & (1 << 31)) *flags |= NEGATIVE_FLAG;
    if ((a > UINT32_MAX - b)) *flags |= CARRY_FLAG;
}

// NOTE or actually perform this, more centralized ALU_add approach? 
// uint32_t alu_add(uint32_t a, uint32_t b, uint32_t *flags) {
//     uint32_t result = a + b;

//     // Set flags based on the result
//     *flags = 0;
//     if (result == 0) *flags |= ZERO_FLAG;
//     if (result & (1 << 31)) *flags |= NEGATIVE_FLAG;
//     if ((a > UINT32_MAX - b)) *flags |= CARRY_FLAG;

//     return result;
// }


uint32_t alu_sub(uint32_t a, uint32_t b) {
    return a-b;
}

void alu_sub_wrapper(uint32_t a, uint32_t b, uint32_t *result, uint32_t *flags) {
    *result = alu_sub(a, b);

    // compiler flag handling. 
    *flags = 0;
    if (*result == 0) *flags |= ZERO_FLAG;
    if (*result & (1 << 31)) *flags |= NEGATIVE_FLAG;
    if (a >= b) *flags |= CARRY_FLAG;
    if (((a ^ b) & (1 << 31)) && ((a ^ *result) & (1 << 31))) {
        *flags |= OVERFLOW_FLAG; 
    }
}

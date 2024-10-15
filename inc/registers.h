/**
 * Will have 2d array for register file 
 */
#ifndef REGISTERS_H
#define REGISTERS_H
#include <stdint.h>

typedef struct {
    uint32_t programCounter;
    uint32_t instructionRegister;

    uint32_t generalRegisters[64];
} registerFile;

extern registerFile regFile;
#endif //REGISTERS_H
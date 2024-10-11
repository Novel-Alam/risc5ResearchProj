/**
 * Will have 2d array for register file 
 */
#pragma once 
#include <stdint.h>

typedef struct {
    uint32_t programCounter;
    uint32_t instructionRegister;

    uint32_t generalRegisters[64];
} registerFile;

extern registerFile regFile;
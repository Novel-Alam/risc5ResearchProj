/**
 * Will have 2d array for register file 
 */
#pragma once 
#include <stdint.h>

typedef struct {
    uint32_t programCounter;

    int x0;    // Zero constant
    int x1;    // Return address
    int x2;    // Stack pointer
    int x3;    // Global pointer
    int x4;    // Thread pointer
    int x5;    // Temporary
    int x6;    // Temporary
    int x7;    // Temporary
    int x8;    // Saved / frame pointer
    int x9;    // Saved register
    int x10;   // Function arg/return value
    int x11;   // Function arg/return value
    int x12;   // Function arg
    int x13;   // Function arg
    int x14;   // Function arg
    int x15;   // Function arg
    int x16;   // Function arg
    int x17;   // Function arg
    int x18;   // Saved register
    int x19;   // Saved register
    int x20;   // Saved register
    int x21;   // Saved register
    int x22;   // Saved register
    int x23;   // Saved register
    int x24;   // Saved register
    int x25;   // Saved register
    int x26;   // Saved register
    int x27;   // Saved register
    int x28;   // Temporary
    int x29;   // Temporary
    int x30;   // Temporary
    int x31;   // Temporary
    float f0;  // FP temporary
    float f1;  // FP temporary
    float f2;  // FP temporary
    float f3;  // FP temporary
    float f4;  // FP temporary
    float f5;  // FP temporary
    float f6;  // FP temporary
    float f7;  // FP temporary
    float f8;  // FP saved register
    float f9;  // FP saved register
    float f10; // FP arg/return value
    float f11; // FP arg/return value
    float f12; // FP arg
    float f13; // FP arg
    float f14; // FP arg
    float f15; // FP arg
    float f16; // FP arg
    float f17; // FP arg
    float f18; // FP saved register
    float f19; // FP saved register
    float f20; // FP saved register
    float f21; // FP saved register
    float f22; // FP saved register
    float f23; // FP saved register
    float f24; // FP saved register
    float f25; // FP saved register
    float f26; // FP saved register
    float f27; // FP saved register
    float f28; // FP temporary
    float f29; // FP temporary
    float f30; // FP temporary
    float f31; // FP temporary
} registerFile;

extern registerFile regFile;
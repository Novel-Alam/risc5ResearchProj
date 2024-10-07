/**
 * text segment starts at 0x00000000 and data segment starts at 0x10010000 
    2d array of memory (2^32 locations by 1 byte) partitioned according to correct architecture
 */

#include <stdint.h>

/* RAM*/
uint8_t ram[UINT32_MAX] = {0};
#include "registers.h"
#include <stdlib.h>


/* Create regFile object */
registerFile regFile;

    
void initRegFile(registerFile *regFile){
    regFile->generalRegisters = (uint32_t*)calloc(32, sizeof(uint32_t));
}


/**
 * void initRegFile(registerFile *regFile){
    regFile = (registerFile*)malloc(sizeof(registerFile));
    regFile->generalRegisters = (uint32_t*)calloc(32, sizeof(uint32_t)); big mistake 
}
 */
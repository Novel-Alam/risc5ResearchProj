#include "../inc/ram.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


void populateDataRAM() {
    for (size_t i = 0; i < UINT32_MAX; i++)
    {
        ramData[i] = 0x000000000;
    }    
}

void populateRAM(char* ASMfile) {
    FILE *asmFile = fopen(ASMfile,"r"); //change file format later
    if(asmFile == NULL) {
        printf("ERROR, INSTRUCTION FILE UNAVALIABLE!!!!"); //TODO: Replace with try-catch block
        return;
    }
    //  char* 

}

uint32_t fetchInstruction(int program){
    return 5;
}
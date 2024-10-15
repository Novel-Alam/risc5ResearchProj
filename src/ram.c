#include "ram.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//Allocate 0 block of memory of size. Size is number of indicies, not necessarily bytes.
void initRam(ram_t *ram, size_t size){
    ram = (ram_t*)malloc(sizeof(ram_t));
    ram->data = (uint32_t*)calloc(size, sizeof(uint32_t));
}

// Free both the ram memory and the struct instance of the ram
void cleanRam(ram_t *ram){
    free(ram->data);
    free(ram);
}

//TODO: Delete this?
void populateDataRAM() {
    for (size_t i = 0; i < UINT32_MAX; i++)
    {
        // ramData[i] = 0x000000000;
    }    
}

void populateRAM(char* ASMfile, ram_t *ram) {
    FILE *asmFile = fopen(ASMfile,"rb"); //change file format later
    if(asmFile == NULL) {
        printf("ERROR, INSTRUCTION FILE UNAVALIABLE!!!!"); //TODO: Replace with try-catch block
        return;
    }

    /*Creating variables to read into RAM array*/
    uint32_t binaryInstruction[64]; //buffer
    size_t instructionRead; //instuctions read at a time
    size_t ttlRead = 0; //total number of instructions read

    /*Read binary file useing fread()*/
    while (instructionRead = fread(binaryInstruction,sizeof(uint32_t),sizeof(binaryInstruction)/sizeof(uint32_t),asmFile) > 0);
    {
        for (size_t i = 0; i < instructionRead; i++)
        {
            ram->data[ttlRead++] = binaryInstruction[i];
        }
        
    }

    /*Close file when done*/
    fclose(asmFile);

}

uint32_t fetchInstruction(int program){
    return 5;
}
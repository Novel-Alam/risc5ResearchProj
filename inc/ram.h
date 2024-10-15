/**
 * text segment starts at 0x00000000 and data segment starts at 0x10010000 
    2d array of memory (2^32 locations by 1 byte) partitioned according to correct architecture
 */

#include <stdint.h>
#include <string.h>




/* RAM*/
// uint32_t ramInstruction[UINT32_MAX];
// uint32_t ramData[UINT32_MAX];

typedef struct{
   uint32_t *data;
}ram_t;
/*
   Open ASM file (parameter)
   populate ram reg with 32 bit instructions
   keep change
*/

/*
   2 seperate functions
   1 that loads instructions from asm file onto ram array
   1 that takes array address as param and returns machine instruction to decoder
*/

/* Function 1*/
/*PREREQ: VALID ASM FILE
  PREREQ: each instruction seperated by \n key
  POSTCOND: ram array populated
*/

void initRam(ram_t *ram, size_t size);
void populateRAM(char* binFileName, ram_t *ram);
void populateDataRAM();

/* Function 2*/
/*POSTCOND: RETURN MACHINE CODE STRING*/

uint32_t fetchInstruction(int program);
/*Function 3*/

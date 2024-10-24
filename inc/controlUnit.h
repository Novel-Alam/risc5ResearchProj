#ifndef PIPELINE_H
#define PIPELINE_H

#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>


/* Handles for each pipeline thread */
extern pthread_t fetchThreadHandle;
extern pthread_t decodeThreadHandle;
extern pthread_t executeThreadHandle;
extern pthread_t memAccessThreadHandle;
extern pthread_t regWriteThreadHandle;

/* Pipes that transfer data from one thread to the next */
extern int pipe_fetch_to_decode[2];
extern int pipe_decode_to_execute[2];
extern int pipe_execute_to_memAccess[2];
extern int pipe_memAccess_to_regWrite[2];

/* Signal that threads will wait on */
extern sigset_t set;

/* Function prototypes for all threads */
void *fetchThread(void *arg);
void *decodeThread(void *arg);
void *executeThread(void *arg);
void *memAccessThread(void *arg);
void *regWriteThread(void *arg);

/* Function prototypes for initialization and cleanup */
void flush_pipe(int pipe_fd);
void cleanup();
int initialPipes();
int initializeSignal();
int initializeThreads();
typedef enum {
    R_TYPE,
    I_TYPE,
    S_TYPE,
    B_TYPE,
    U_TYPE,
    J_TYPE
}INSTR_TYPE;


/**
 * Control unit fetch state sets operation to one of these in this enums
 */
enum microOp {
     // Arithmetic operations
     OP_ADD,      // Addition
     OP_SUB,      // Subtraction
     OP_XOR,      // Bitwise XOR
     OP_OR,       // Bitwise OR
     OP_AND,      // Bitwise AND
     OP_SLL,      // Shift left logical
     OP_SRL,      // Shift right logical
     OP_SRA,      // Shift right arithmetic
     OP_SLT,      // Set less than (signed comparison)
     OP_SLTU,     // Set less than unsigned (unsigned comparison)

     // Immediate arithmetic operations
     OP_ADDI,     // Addition with immediate
     OP_XORI,     // Bitwise XOR with immediate
     OP_ORI,      // Bitwise OR with immediate
     OP_ANDI,     // Bitwise AND with immediate
     OP_SLLI,     // Shift left logical with immediate
     OP_SRLI,     // Shift right logical with immediate
     OP_SRAI,     // Shift right arithmetic with immediate
     OP_SLTI,     // Set less than immediate (signed comparison)
     OP_SLTIU,    // Set less than unsigned immediate (unsigned comparison)

     // Load instructions
     OP_LB,       // Load byte (signed)
     OP_LH,       // Load halfword (signed)
     OP_LW,       // Load word (signed)
     OP_LBU,      // Load byte unsigned
     OP_LHU,      // Load halfword unsigned

     // Store instructions
     OP_SB,       // Store byte
     OP_SH,       // Store halfword
     OP_SW,       // Store word

     // Branch instructions
     OP_BEQ,      // Branch if equal
     OP_BNE,      // Branch if not equal
     OP_BLT,      // Branch if less than (signed)
     OP_BGE,      // Branch if greater than or equal (signed)
     OP_BLTU,     // Branch if less than (unsigned)
     OP_BGEU,     // Branch if greater than or equal (unsigned)

     // Jump instructions
     OP_JAL,      // Jump and link
     OP_JALR,     // Jump and link register

     // Upper immediate operations
     OP_LUI,      // Load upper immediate
     OP_AUIPC,    // Add upper immediate to PC

     // System instructions
     OP_ECALL,    // Environment call
     OP_EBREAK,   // Environment break

    // Multiply extension (RV32M)
     OP_MUL,      // Multiply (low 32 bits)
     OP_MULH,     // Multiply high (signed × signed)
     OP_MULSU,    // Multiply high (signed × unsigned)
     OP_MULU,     // Multiply high (unsigned × unsigned)
     OP_DIV,      // Division (signed)
     OP_DIVU,     // Division (unsigned)
     OP_REM,      // Remainder (signed)
     OP_REMU,     // Remainder (unsigned)

    // Atomic extension (RV32A)
    OP_LRW,       // Load reserved word
    OP_SCW,       // Store conditional word
    OP_AMOSWAPW,  // Atomic swap word
    OP_AMOADDW,   // Atomic add word
    OP_AMOANDW,   // Atomic AND word
    OP_AMOORW,    // Atomic OR word
    OP_AMOXORW,   // Atomic XOR word
    OP_AMOMAXW,   // Atomic maximum word (signed)
    OP_AMOMINW    // Atomic minimum word (signed)
};

//change this fucking monstrosity

typedef struct {
    INSTR_TYPE instruction_type;  // Determines which union field to use
    uint8_t microOp;              // Decoded instruction mnemonic
    uint8_t opcode;               // Common across all instruction types

    // Union of different instruction formats
    union {
        // R-type instruction fields
        struct {
            uint8_t rd;
            uint8_t funct3;
            uint8_t rs1;
            uint8_t rs2;
            uint8_t funct7;
        } r_type;

        // I-type instruction fields
        struct {
            uint8_t rd;
            uint8_t funct3;
            uint8_t rs1;
            uint16_t imm12;  // Immediate is 12 bits for I-type
        } i_type;

        // S-type instruction fields
        struct {
            uint8_t funct3;
            uint8_t rs1;
            uint8_t rs2;
            uint16_t imm12;  // Immediate is split into parts
        } s_type;

        // B-type instruction fields
        struct {
            uint8_t funct3;
            uint8_t rs1;
            uint8_t rs2;
            uint16_t imm12;  // Immediate is 12 bits (split for branching)
        } b_type;

        // U-type instruction fields
        struct {
            uint8_t rd;
            uint32_t imm20;  // Immediate is 20 bits for U-type
        } u_type;

        // J-type instruction fields
        struct {
            uint8_t rd;
            uint32_t imm20;  // Immediate is 20 bits for J-type
        } j_type;

    } instrFields;  // Union to store different instruction formats

} decodedFields;


typedef enum {
    FETCH,
    DECODE,
    EXECUTE,
    MEM_ACCESS,
    REG_WRITE_BACK
} currentStage;


/* Signal handler for SIGINT (Ctrl+C) */
void sigint_handler(int sig);

INSTR_TYPE get_Instr_Type(uint8_t opcode);

#define MSB_8BIT 0x80 //10000000
#endif // PIPELINE_H
#include "controlUnit.h"
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
#include "ram.h"
#include "registers.h"
#include "alu.h"

#define INSTRUCTION_TO_RD(instructionToDecode) ((instructionToDecode >> 7) & 0b11111)
#define INSTRUCTION_TO_FUNCT3(instructionToDecode) ((instructionToDecode >> 12) & 0b111)
#define INSTRUCTION_TO_RS1(instruction) (((instruction) >> 15) & 0b1111)
#define INSTRUCTION_TO_RS2(instruction) (((instruction) >> 20) & 0b1111)
#define INSTRUCTION_TO_FUNCT7(instruction) (((instruction) >> 25) & 0b1111111)

#define INSTRUCTION_TO_IMMI(instructionToDecode) ( (instructionToDecode >> 20) && 0b1111111)

#define LOGICAL_I_TYPE 0b0010011
#define LOAD_I_TYPE 0b0000011
#define JAL_I_TYPE 0b1100111
#define FENCE_I_TYPE 0b0001111 //Memory barrier instructions 



/* Handles for each pipeline thread */
pthread_t fetchThreadHandle;
pthread_t decodeThreadHandle;
pthread_t executeThreadHandle;
pthread_t memAccessThreadHandle;
pthread_t regWriteThreadHandle;

/* Pipes that transfer data from one thread to the next */
int pipe_fetch_to_decode[2];
int pipe_decode_to_execute[2];
int pipe_execute_to_memAccess[2];
int pipe_memAccess_to_regWrite[2];

/* Signal that threads will wait on */
sigset_t set;

/* Holds the current state of the system */
currentStage currStage = FETCH;



/* Function prototypes for all threads */
void *fetchThread(void *arg);
void *decodeThread(void *arg);
void *executeThread(void *arg);
void *memAccessThread(void *arg);
void *regWriteThread(void *arg);

void flush_pipe(int pipe_fd) {
    char buf[1024];
    while (read(pipe_fd, buf, sizeof(buf)) > 0) {
        // Continue reading until the pipe is empty
    }
}

void cleanup() {
    printf("Flushing pipes...\n");

    /* Flush all the pipes */
    flush_pipe(pipe_fetch_to_decode[0]);
    flush_pipe(pipe_decode_to_execute[0]);
    flush_pipe(pipe_execute_to_memAccess[0]);
    flush_pipe(pipe_memAccess_to_regWrite[0]);

    /* Cancel threads */
    pthread_cancel(fetchThreadHandle);
    pthread_cancel(decodeThreadHandle);
    pthread_cancel(executeThreadHandle);
    pthread_cancel(memAccessThreadHandle);
    pthread_cancel(regWriteThreadHandle);

    /* Close all pipes */
    close(pipe_fetch_to_decode[0]);
    close(pipe_fetch_to_decode[1]);
    close(pipe_decode_to_execute[0]);
    close(pipe_decode_to_execute[1]);
    close(pipe_execute_to_memAccess[0]);
    close(pipe_execute_to_memAccess[1]);
    close(pipe_memAccess_to_regWrite[0]);
    close(pipe_memAccess_to_regWrite[1]);

    printf("Cleanup done. Exiting program.\n");
    exit(0);
}
/* Signal handler for SIGINT (Ctrl+C) */
void sigint_handler(int sig) {
    printf("SIGINT received. Initiating cleanup...\n");
    cleanup();
}

/* Initializes all pipes */
int initialPipes() {
    if (pipe(pipe_fetch_to_decode) == -1 ||
        pipe(pipe_decode_to_execute) == -1 ||
        pipe(pipe_execute_to_memAccess) == -1 ||
        pipe(pipe_memAccess_to_regWrite) == -1) {
        perror("Pipe creation error");
        exit(EXIT_FAILURE);
    }

    /* Convert read ends to non-blocking */
    int flags;
    flags = fcntl(pipe_fetch_to_decode[0], F_GETFL, 0);
    fcntl(pipe_fetch_to_decode[0], F_SETFL, flags | O_NONBLOCK); /* or the nonblock bit*/

    flags = fcntl(pipe_decode_to_execute[0], F_GETFL, 0);
    fcntl(pipe_decode_to_execute[0], F_SETFL, flags | O_NONBLOCK);

    flags = fcntl(pipe_execute_to_memAccess[0], F_GETFL, 0);
    fcntl(pipe_execute_to_memAccess[0], F_SETFL, flags | O_NONBLOCK);

    flags = fcntl(pipe_memAccess_to_regWrite[0], F_GETFL, 0);
    fcntl(pipe_memAccess_to_regWrite[0], F_SETFL, flags | O_NONBLOCK);
}

/* Initializes signal handling */
int initializeSignal() {
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
}

/* Initializes all threads */
int initializeThreads() {
    pthread_create(&fetchThreadHandle, NULL, fetchThread, (void*)&set);
    pthread_create(&decodeThreadHandle, NULL, decodeThread, (void*)&set);
    pthread_create(&executeThreadHandle, NULL, executeThread, (void*)&set);
    pthread_create(&memAccessThreadHandle, NULL, memAccessThread, (void*)&set);
    pthread_create(&regWriteThreadHandle, NULL, regWriteThread, (void*)&set);
}

/* Fetch Thread */
void *fetchThread(void *arg) {
    sigset_t *set = (sigset_t *)arg;
    int signal;
    

    for (;;) {
        /* Block on signal */
        sigwait(set, &signal);

        if (signal == SIGUSR1 && currStage == FETCH) {
            printf("Fetch Thread\n");

            /* Fetch instruction from Instruction memory using program counter */
            regFile.instructionRegister = fetchInstruction( regFile.programCounter );

            /* Incriment Program counter by 4 bytes (32 bits) */
            regFile.programCounter += 32;

            /* Pass the fetched uint32_t instruction */
            write( pipe_fetch_to_decode[1], (uint32_t*)regFile.instructionRegister, sizeof(regFile.instructionRegister) );
        } else {
            perror("Different signal received");
        }
    }
}

INSTR_TYPE get_Instr_Type(uint8_t opcode){
    if ((opcode & MSB_8BIT) != 0){
        printf("Invalid Instruction Type: %d", opcode);
        exit(1);
    }
    switch(opcode){
        case(0b0110011):
            return R_TYPE;
        case(0b0000011):
            return I_TYPE;
        case(0b0010011):
            return I_TYPE;
        case(0b1100111):
            return I_TYPE;
        case(0b0100011):
            return S_TYPE;
        case(0b1100011):
            return B_TYPE;
        case(0b0110111):
            return U_TYPE;
        case(0b1101111):
            return J_TYPE;
        default:
            printf("OPCode not recognized: %d", opcode);
            exit(1);
    }
}


/* Decode Thread */
void *decodeThread(void *arg) {
    sigset_t *set = (sigset_t *)arg;
    int signal;
    char read_buf[5000];


    for (;;) {
        /* Block on signal */
        sigwait(set, &signal);

        if (signal == SIGUSR1 && read(pipe_fetch_to_decode[0], read_buf, sizeof(read_buf)) && (currStage == DECODE) ) {
            printf("Decode Thread: %s\n", read_buf);

            /* Instructions to decode */
            uint32_t instructionToDecode;

            decodedFields df = {0};
            df.opcode = instructionToDecode & 0b1111111; /* extract opcode*/
            df.instruction_type = get_Instr_Type(df.opcode);//we have 3 i types btw each has diff opcode
            INSTR_TYPE type = df.instruction_type;
            /* Determine type of instruction */
            switch(type){
                case R_TYPE://Rtype
                    df.instrFields.r_type.rd = INSTRUCTION_TO_RD(instructionToDecode);
                    df.instrFields.r_type.funct3 = INSTRUCTION_TO_FUNCT3(instructionToDecode);
                    df.instrFields.r_type.rs1 = INSTRUCTION_TO_RS1(instructionToDecode);
                    df.instrFields.r_type.rs2 = INSTRUCTION_TO_RS2(instructionToDecode);
                    df.instrFields.r_type.funct7 = INSTRUCTION_TO_FUNCT7(instructionToDecode);
                    //bro missed like 10 break statements

                    /* Determine exact instruction*/
                    switch (df.instrFields.r_type.funct3) {
                        case 0x0:
                            switch (df.instrFields.r_type.funct7) {
                                case 0x00:
                                    df.microOp = OP_ADD;
                                    break;
                                case 0x20:
                                    df.microOp = OP_SUB;
                                    break;
                                default:
                                    perror("Incorrect funct7");
                                    break;
                            }
                            break;
                        case 0x4:
                            switch (df.instrFields.r_type.funct7) {
                                case 0x00:
                                    df.microOp = OP_XOR;
                                    break;
                                default:
                                    perror("Incorrect funct7");

                            }
                            break;
                        case 0x6:
                            switch (df.instrFields.r_type.funct7) {
                                case 0x00:
                                    df.microOp = OP_OR;
                                    break;
                                default:
                                    perror("Incorrect funct7");
                            }
                            break;
                        case 0x7:
                            switch (df.instrFields.r_type.funct7) {
                                case 0x00:
                                   df.microOp = OP_AND;
                                   break;
                                default:
                                    perror("Incorrect funct7");
                            }
                            break;
                        case 0x1:
                            switch (df.instrFields.r_type.funct7) {
                                case 0x00:
                                    df.microOp = OP_SLL;
                                    break;
                                case 0x01://mul extention
                                    switch(df.instrFields.r_type.funct3){
                                        case 0x0:
                                            df.microOp = OP_MUL;
                                            break;
                                        case 0x1:
                                            df.microOp = OP_MULH;
                                            break;
                                        case 0x2:
                                            df.microOp = OP_MULSU;
                                            break;
                                        case 0x3:
                                            df.microOp = OP_MULU;
                                            break;
                                        case 0x4:
                                            df.microOp = OP_DIV;
                                            break;
                                        case 0x5:
                                            df.microOp = OP_DIVU;
                                            break;
                                        case 0x6:
                                            df.microOp = OP_REM;
                                            break;
                                        case 0x7:
                                            df.microOp = OP_REMU;
                                            break;
                                        default:
                                            perror("404 r type mul op not found");
                                            break;
                                    }
                                    break;
                                default:
                                    perror("Incorrect funct7");

                            }
                            break;
                        case 0x5:
                            switch (df.instrFields.r_type.funct7) {
                                case 0x00:
                                    df.microOp = OP_SRL;
                                    break;
                                case 0x20:
                                    df.microOp = OP_SRA;
                                    break;
                                default:
                                    perror("Incorrect funct7");

                            }
                            break;
                        case 0x2:
                            switch (df.instrFields.r_type.funct7) {
                                case 0x00:
                                    df.microOp = OP_SLT;
                                    break;
                                default:
                                    perror("Incorrect funct7");

                            }
                            break;
                        case 0x3:
                            switch (df.instrFields.r_type.funct7) {
                                case 0x00:
                                    df.microOp = OP_SLTU;
                                    break;
                                default:
                                    perror("Incorrect funct7");

                            }
                            break;
                        default:
                            perror("incorect funct3 bits");
                    }
                case I_TYPE:
                    df.instrFields.i_type.rd = INSTRUCTION_TO_RD(instructionToDecode);
                    df.instrFields.i_type.funct3 = INSTRUCTION_TO_FUNCT3(instructionToDecode);
                    df.instrFields.i_type.rs1 = INSTRUCTION_TO_RS1(instructionToDecode);
                    df.instrFields.i_type.imm12 = INSTRUCTION_TO_IMMI(instructionToDecode);
                    // Logical I-type
                    if (df.opcode == LOGICAL_I_TYPE){
                        switch(df.instrFields.i_type.funct3){
                            case 0x0:
                                df.microOp = OP_ADDI;
                                break;
                            case 0x1:
                                df.microOp = OP_SLLI;
                                break;
                            case 0x2:
                                df.microOp = OP_SLTI;
                                break;
                            case 0x3:
                                df.microOp = OP_SLTIU;
                                break;
                            case 0x4:
                                df.microOp = OP_XORI;
                                break;
                            case 0x5:
                                switch(df.instrFields.i_type.funct3){
                                    case 0x0 :
                                        df.microOp = OP_SRLI;
                                        break;
                                    case 0x20 :
                                        df.microOp = OP_SRAI;
                                        break;
                                    default:
                                        perror("illegal imm for srli and srai differentiation\n");
                                }
                                break;
                            case 0x6:
                                df.microOp = OP_ORI;
                                break;
                            case 0x7:
                                df.microOp = OP_ANDI;
                                break;
                            default:
                                perror("404 I type funct 3 not found\n");
                                printf("%x LoadI-type instruction not found", df.opcode);
                            }
                    }
                    //ecall/ebreak
                    else if(df.opcode == 0b1110011){
                        switch(df.instrFields.i_type.imm12){
                            case 0x0:
                                df.microOp = OP_ECALL;
                                break;
                            case 0x1:
                                df.microOp = OP_EBREAK;//idk how we're gonna implement this lmao
                                break;
                            default:
                                perror("ecall/break error");//was spelled peerror lmao
                        }
                    }
                    //Load I-type
                    else if(df.opcode == LOAD_I_TYPE){
                        switch(df.instrFields.i_type.funct3){
                            case 0x0:
                                df.microOp = OP_LB;
                                break;
                            case 0x1:
                                df.microOp = OP_LH;
                                break;
                            case 0x2:
                                df.microOp = OP_LW;
                                break;
                            case 0x4:
                                df.microOp = OP_LBU;
                                break;
                            case 0x5:
                                df.microOp = OP_LHU;
                                break;
                            default:
                                printf("%x Load I-type instruction not found", df.opcode);
                                break;
                        }
                    }

                    else if(df.opcode== JAL_I_TYPE) {
                        switch(df.instrFields.i_type.funct3) {
                            case 0x0:
                                df.microOp = OP_JAL;
                                break;
                            default:
                                perror("404 I type funct 3 not found\n");
                                break;
                        }
                    }
                    break;
                case S_TYPE:

                    break;
                case B_TYPE:

                    break;
                case U_TYPE:
                
                    break;
                case J_TYPE:
                    
                    break;

                case 0b0101111://atomic extension
                    break;
                default:
                printf("Instruction type: %d not found", df.instruction_type);
                    break;
            }


            /* Pass df to pipe */
            write(pipe_decode_to_execute[1], &df, sizeof(df));

            

        } else {
            perror("Nothing to decode yet");
        }
    }
}

/* Execute Thread */
void *executeThread(void *arg) {
    sigset_t *set = (sigset_t *)arg;
    int signal;
    
    /* Will be populated from data from pipe*/
    decodedFields df = {0};

    uint32_t aluResult;

    for (;;) {
        /* Block on signal */
        sigwait(set, &signal);

        if (signal == SIGUSR1 && read(pipe_decode_to_execute[0], &df, sizeof(df)) && (currStage == EXECUTE) ){
            /* Execute the command */
            switch (df.microOp) {
                case OP_ADD: 
                    aluResult =  alu_add(regFile.generalRegisters[df.instrFields.r_type.rs1], regFile.generalRegisters[df.instrFields.r_type.rs2]);
                    break;
                case OP_ADDI:
                    aluResult = alu_add(regFile.generalRegisters[df.instrFields.i_type.rs1], (uint32_t)df.instrFields.i_type.imm12);
                case OP_SUB:
                    aluResult =  alu_sub(regFile.generalRegisters[df.instrFields.r_type.rs1], regFile.generalRegisters[df.instrFields.r_type.rs2]);
                    break;

                /* add more later*/
                default:
                    perror("Instruction no implimented yet");
            }
            

            /* pass the result of the alu operation */
            write(pipe_execute_to_memAccess[1], &aluResult, sizeof(aluResult));
        } else {
            perror("Nothing to execute yet");
        }
    }
}

/* Memory Access Thread */
void *memAccessThread(void *arg) {
    sigset_t *set = (sigset_t *)arg;
    int signal;
    uint32_t valueFromExecute;

    for (;;) {
        /* Block on signal */
        sigwait(set, &signal);

        if (signal == SIGUSR1 && read(pipe_execute_to_memAccess[0], &valueFromExecute, sizeof(valueFromExecute)) && (currStage == MEM_ACCESS) ) {
            printf("Memory Access Thread: %s\n", valueFromExecute);
            char message[] = "Memory accessed";
            
            write(pipe_memAccess_to_regWrite[1], &valueFromExecute, sizeof(valueFromExecute));
        } else {
            perror("Nothing to access in memory yet");
        }
    }
}

/* Register Write Thread */
void *regWriteThread(void *arg) {
    sigset_t *set = (sigset_t *)arg;
    int signal;
    uint32_t valueFromExecute;

    for (;;) {
        /* Block on signal */
        sigwait(set, &signal);

        if (signal == SIGUSR1 && read(pipe_memAccess_to_regWrite[0], &valueFromExecute, sizeof(valueFromExecute)) && (currStage == REG_WRITE_BACK) ) {
            printf("Register Write Thread: %s\n", valueFromExecute);
            regFile.generalRegisters[2];
        } else {
            perror("Nothing to write to register yet");
        }
    }

}


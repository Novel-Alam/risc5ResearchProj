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
    fcntl(pipe_fetch_to_decode[0], F_SETFL, flags | O_NONBLOCK);

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

        if (signal == SIGUSR1) {
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

    typedef struct {
        INSTR_TYPE instruction_type;
        uint8_t opcode;
        uint8_t rd;
        uint8_t funct3;
        uint8_t rs1;
        uint8_t rs2;
        uint8_t funct7;
        uint8_t microOp; /* Decoded instruction mnemonic */
        //RV32 atomic extension use
        uint8_t funct5 = NULL;
        uint8_t aq = NULL;
        uint8_t rl = NULL;
    } decodedFields;


/* Decode Thread */
void *decodeThread(void *arg) {
    sigset_t *set = (sigset_t *)arg;
    int signal;
    char read_buf[5000];


    for (;;) {
        /* Block on signal */
        sigwait(set, &signal);

        if (signal == SIGUSR1 && read(pipe_fetch_to_decode[0], read_buf, sizeof(read_buf)) > 0) {
            printf("Decode Thread: %s\n", read_buf);

            /* Instructions to decode */
            uint32_t instructionToDecode;

            decodedFields df = {0};
            df.opcode = instructionToDecode & 0b1111111; /* extract opcode*/

            /* Determine type of instruction */
            switch(df.opcode){
                case 0b0110011://Rtype
                    df.instruction_type = R_TYPE;
                    df.rd = (instructionToDecode >> 7) & 0b11111;
                    df.funct3 = (instructionToDecode >> 12) & 0b111;
                    df.rs1 = (instructionToDecode >> 15) & 0b1111;
                    df.rs2 = (instructionToDecode >> 20) & 0b1111;
                    df.funct7 = (instructionToDecode >> 25) & 0b1111111;
                    break;//wtf is this

                    /* Determine exact instruction*/
                    switch (df.funct3) {
                        case 0x0:
                            switch (df.funct7) {
                                case 0x00:
                                    df.microOp = OP_ADD;
                                    break;
                                case 0x20:
                                    df.microOp = OP_SUB;
                                    break;
                                default:
                                    perror("Incorrect funct7");

                            }
                        case 0x4:
                            switch (df.funct7) {
                                case 0x00:
                                    df.microOp = OP_XOR;
                                default:
                                    perror("Incorrect funct7");

                            }
                        case 0x6:
                            switch (df.funct7) {
                                case 0x00:
                                    df.microOp = OP_OR;
                                default:
                                    perror("Incorrect funct7");

                            }
                        case 0x7:
                            switch (df.funct7) {
                                case 0x00:
                                   df.microOp = OP_AND; 
                                default:
                                    perror("Incorrect funct7");

                            }
                        case 0x1:
                            switch (df.funct7) {
                                case 0x00:
                                    df.microOp = OP_SLL;
                                default:
                                    perror("Incorrect funct7");

                            }
                        case 0x5:
                            switch (df.funct7) {
                                case 0x00:
                                    df.microOp = OP_SRL;
                                case 0x20:
                                    df.microOp = OP_SRA;
                                default:
                                    perror("Incorrect funct7");

                            }
                        case 0x2:
                            switch (df.funct7) {
                                case 0x00:
                                    df.microOp = OP_SLT;
                                default:
                                    perror("Incorrect funct7");

                            }
                        case 0x3:
                            switch (df.funct7) {
                                case 0x00:
                                    df.microOp = OP_SLTU;
                                default:
                                    perror("Incorrect funct7");

                            }

                        default:
                            perror("incorect funct3 bits");
                    }
                //mul R type
                case 0b0110011:
                    df.instruction_type = I_TYPE;
                    df.rd = (instructionToDecode >> 7) & 0b11111;
                    df.funct3 = (instructionToDecode >> 12) & 0b111;
                    df.rs1 = (instructionToDecode >> 15) & 0b1111;
                    df.rs2 = (instructionToDecode >> 20) & 0b1111;
                    df.funct7 = (instructionToDecode >> 25) & 0b1111111;
                    if(df.funct7 != 0x1){
                        perror("mul instruction got fucked");
                    }
                    switch(df.funct3){
                        case 0x0:
                            df.microOP = OP_MUL;
                            break;
                        case 0x1:
                            df.microOP = OP_MULH;
                            break;
                        case 0x2:
                            df.microOP = MULSU;
                            break;
                        case 0x3:
                            df.microOP = MULU;
                            break;
                        case 0x4:
                            df.microOP = OP_DIV;
                            break;
                        case 0x5:
                            df.microOP = OP_DIVU;
                            break;
                        case 0x6:
                            df.microOP = OP_REM;
                            break;
                        case 0x7:
                            df.microOP = OP_REMU;
                            break;
                        default:
                            perror("404 r type mul op not found");
                    }
                    break;
                /* logical I type */
                case 0b0010011:
                    df.instruction_type = I_TYPE;
                    df.rd = (instructionToDecode >> 7) & 0b11111;
                    df.funct3 = (instructionToDecode >> 12) & 0b111;
                    df.rs1 = (instructionToDecode >> 15) & 0b1111;
                    df.rs2 = (instructionToDecode >> 20) & 0b1111;
                    df.funct7 = (instructionToDecode >> 25) & 0b1111111;
                    switch(df.funct3){
                        case 0x0:
                            df.microOp = OP_ADDI;
                            break;
                        case 0x1:
                            if(df.funct7 != 0x0){
                            perror("slli overflow error");
                        }
                            df.microOp = OP_SLLI;
                            break;
                        case 0x2:
                            df.microOp = OP_SLTI;
                            break;
                        case 0x3:
                            df.microOP = OP_SLTIU;
                            break;
                        case 0x4:
                            df.microOP = OP_XORI;
                            break;
                        case 0x5:
                            switch(df.funct7){
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
                            df.microOP = OP_ORI;
                            break;
                        case 0x7:
                            df.microOP = OP_ANDI;
                            break;
                        default:
                            perror("404 I type funct 3 not found\n");
                    }
                    break;
                /*load I Type*/
                case 0b0000011:
                    df.instruction_type = I_TYPE;
                    df.rd = (instructionToDecode >> 7) & 0b11111;
                    df.funct3 = (instructionToDecode >> 12) & 0b111;
                    df.rs1 = (instructionToDecode >> 15) & 0b1111;
                    df.rs2 = (instructionToDecode >> 20) & 0b1111;
                    df.funct7 = (instructionToDecode >> 25) & 0b1111111;
                    switch(df.funct3){
                        case 0x0:
                            df.microOP = OP_LB;
                            break;
                        case 0x1:
                            df.microOP = OP_LH;
                            break;
                        case 0x2:
                            df.microOP = OP_LW;
                            break;
                        case 0x4:
                            df.microOP = OP_LBU;
                            break;
                        case 0x5:
                            df.microOP = OP_LHU;
                            break;
                        default:
                            perror("load I Type funct3 error");
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

        if (signal == SIGUSR1 && read(pipe_decode_to_execute[0], &df, sizeof(df)) > 0) {
            /* Execute the command */
            switch (df.microOp) {
                case OP_ADD: 
                    aluResult =  alu_add(regFile.generalRegisters[df.rs1], regFile.generalRegisters[df.rs2]);
                    break;
                case OP_SUB:
                    aluResult =  alu_sub(regFile.generalRegisters[df.rs1], regFile.generalRegisters[df.rs2]);
                    break;

                /* add more later*/
                default:
                    perror("Instruciton no implimented yet");
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

        if (signal == SIGUSR1 && read(pipe_execute_to_memAccess[0], &valueFromExecute, sizeof(valueFromExecute)) > 0) {
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

        if (signal == SIGUSR1 && read(pipe_memAccess_to_regWrite[0], &valueFromExecute, sizeof(valueFromExecute)) > 0) {
            printf("Register Write Thread: %s\n", valueFromExecute);
            regFile.generalRegisters[2];
        } else {
            perror("Nothing to write to register yet");
        }
    }

}


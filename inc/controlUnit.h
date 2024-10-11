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
/* Signal handler for SIGINT (Ctrl+C) */
void sigint_handler(int sig);

INSTR_TYPE get_Instr_Type(uint8_t opcode);

#define MSB_8BIT 0x80 //10000000
#endif // PIPELINE_H
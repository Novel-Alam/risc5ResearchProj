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
            char message[] = "Instruction from Fetch";
            write(pipe_fetch_to_decode[1], message, sizeof(message));
        } else {
            perror("Different signal received");
        }
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

        if (signal == SIGUSR1 && read(pipe_fetch_to_decode[0], read_buf, sizeof(read_buf)) > 0) {
            printf("Decode Thread: %s\n", read_buf);
            char message[] = "Instruction decoded";
            write(pipe_decode_to_execute[1], message, sizeof(message));
        } else {
            perror("Nothing to decode yet");
        }
    }
}

/* Execute Thread */
void *executeThread(void *arg) {
    sigset_t *set = (sigset_t *)arg;
    int signal;
    char read_buf[5000];

    for (;;) {
        /* Block on signal */
        sigwait(set, &signal);

        if (signal == SIGUSR1 && read(pipe_decode_to_execute[0], read_buf, sizeof(read_buf)) > 0) {
            printf("Execute Thread: %s\n", read_buf);
            char message[] = "Instruction executed";
            write(pipe_execute_to_memAccess[1], message, sizeof(message));
        } else {
            perror("Nothing to execute yet");
        }
    }
}

/* Memory Access Thread */
void *memAccessThread(void *arg) {
    sigset_t *set = (sigset_t *)arg;
    int signal;
    char read_buf[5000];

    for (;;) {
        /* Block on signal */
        sigwait(set, &signal);

        if (signal == SIGUSR1 && read(pipe_execute_to_memAccess[0], read_buf, sizeof(read_buf)) > 0) {
            printf("Memory Access Thread: %s\n", read_buf);
            char message[] = "Memory accessed";
            write(pipe_memAccess_to_regWrite[1], message, sizeof(message));
        } else {
            perror("Nothing to access in memory yet");
        }
    }
}

/* Register Write Thread */
void *regWriteThread(void *arg) {
    sigset_t *set = (sigset_t *)arg;
    int signal;
    char read_buf[5000];

    for (;;) {
        /* Block on signal */
        sigwait(set, &signal);

        if (signal == SIGUSR1 && read(pipe_memAccess_to_regWrite[0], read_buf, sizeof(read_buf)) > 0) {
            printf("Register Write Thread: %s\n", read_buf);
        } else {
            perror("Nothing to write to register yet");
        }
    }

}


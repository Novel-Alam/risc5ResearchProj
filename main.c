#include "controlUnit.h"
/* Main function */
int main() {
    signal(SIGINT, sigint_handler);  // Register SIGINT handler
    initializeSignal();
    initialPipes();
    initializeThreads();

    for (;;) {
        printf("\n\n\n");
        /* Simulate signal delivery */
        pthread_kill(regWriteThreadHandle, SIGUSR1);
        usleep(100);
        pthread_kill(memAccessThreadHandle, SIGUSR1);
        usleep(100);
        pthread_kill(executeThreadHandle, SIGUSR1);
        usleep(100);
        pthread_kill(decodeThreadHandle, SIGUSR1);
        usleep(100);
        pthread_kill(fetchThreadHandle, SIGUSR1);
        sleep(5);  // Delay to simulate timing
        
    }

    /* Join threads (optional) */
    pthread_join(fetchThreadHandle, NULL);
    pthread_join(decodeThreadHandle, NULL);
    pthread_join(executeThreadHandle, NULL);
    pthread_join(memAccessThreadHandle, NULL);
    pthread_join(regWriteThreadHandle, NULL);

    return 0;
}
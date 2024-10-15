/**
 * Will have a thread that will send a signal (represents rising edge)
 */
#ifndef CLOCK_H
#define CLOCK_H

/**
 * @brief This fucntion will send a signal to fetch, decode, execute, memaccess, and writeback threads causing them to run
 */
void sendRisingEdge();

#endif //CLOCK_H
#ifndef SERVERFUNCS_H
#define SERVERFUNCS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define PORT 12345
#define SHM_SIZE 1024
#define SHAREDMEMORUN 1 // Share memory flag for running.
#define SOCKETRUN 1 // Socket flag for running.

/**
 * @brief Monitors changes in shared memory and sends updates to the client socket.
 * 
 * This function continuously monitors the shared memory pointed to by shm_ptr.
 * When changes are detected (compared to the previous content), it prints the new
 * content and sends it back to the client using the provided socket.
 * 
 * @param shm_ptr Pointer to the shared memory area to monitor.
 * @param client_sock Client socket descriptor to send updated content.
 */
void monitorSharedMemory(char *shm_ptr, int client_sock);

/**
 * @brief Handles incoming data from the client and stores it in shared memory.
 * 
 * This function continuously receives data from the client, parses it, and stores the parsed values in shared memory.
 * It terminates upon client disconnection or receive error.
 * 
 * @param new_sock Client socket descriptor for receiving data.
 * @param shm_ptr Pointer to the shared memory segment.
 */
void handleClientData(int new_sock, char *shm_ptr) ;
#endif // SERVERFUNCS_H
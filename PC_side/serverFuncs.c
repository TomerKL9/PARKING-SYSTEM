#include "serverFuncs.h"


void monitorSharedMemory(char *shm_ptr, int client_sock) {
    char prev_shm[SHM_SIZE] = ""; // Variable to store previous shared memory content

    while (SHAREDMEMORUN) {
        if (strcmp(shm_ptr, prev_shm) != 0) { // If the shared memory has changed.
            strcpy(prev_shm, shm_ptr); // Update previous shared memory content
            printf("Shared memory content: '%s'\n", shm_ptr);
            // Send updated shared memory content back to client
            send(client_sock, shm_ptr, strlen(shm_ptr), 0);
        }
        usleep(100000); // Sleep for 100 ms before checking again
    }
}


void handleClientData(int new_sock, char *shm_ptr) {
    int recv_size; // Size of received data
    char buffer[1024]; // Buffer for incoming data

    while (SOCKETRUN) {
        recv_size = recv(new_sock, buffer, sizeof(buffer) - 1, 0); // Leave space for null terminator
        if (recv_size > 0) {
            buffer[recv_size] = '\0'; // Null-terminate the received string
            uint8_t receivedValue;
            uint8_t direction;
            sscanf(buffer, "%c%c", &receivedValue, &direction);
            printf("Received: %d%c\n", receivedValue, direction);

            // Store the received characters in shared memory
            snprintf(shm_ptr, SHM_SIZE, "%d%c", receivedValue, direction);
        } else if (recv_size == 0) {
            printf("Client disconnected\n");
            break;
        } else if (recv_size < 0) {
            perror("Receive failed");
            break;
        }
    }
}
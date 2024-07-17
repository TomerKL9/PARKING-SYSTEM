#include "serverFuncs.h"


/**
 * @brief Main function that initializes the server, handles client connections,
 *        and manages shared memory.
 * 
 * The server creates a socket, binds it to a port, and listens for incoming connections.
 * Upon accepting a connection, it forks a process:
 * - The child process monitors changes in shared memory and sends updates to the client.
 * - The parent process handles incoming data from the client and stores it in shared memory.
 * 
 * @return Returns 0 on success, non-zero on failure.
 */
int main() {
    int sockfd, new_sock; // Socket descriptors
    struct sockaddr_in server, client; // Server and client address structures
    socklen_t client_len; // Size of client address structure
    int recv_size; // Size of received data
    char buffer[1024]; // Buffer for incoming data
    
    // Shared memory variables
    int shmid; // Shared memory ID
    key_t key; // Key for shared memory
    char *shm_ptr; // Pointer to shared memory
    
    // Generate a unique key for the shared memory segment
    key = ftok("/tmp", 'S');
    if (key == -1) {
        perror("ftok");
        return 1;
    }

    // Create the shared memory segment
    shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    // Attach the shared memory segment
    shm_ptr = shmat(shmid, NULL, 0);
    if (shm_ptr == (char *) -1) {
        perror("shmat");
        return 1;
    }

    printf("Shared memory server ready\n");

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        return 1;
    }

    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // Bind
    if (bind(sockfd, (struct sockaddr*)&server, sizeof(server)) == -1) {
        perror("Bind failed");
        close(sockfd);
        return 1;
    }

    // Listen
    if (listen(sockfd, 3) == -1) {
        perror("Listen failed");
        close(sockfd);
        return 1;
    }

    printf("Server listening on port %d\n", PORT);

    // Accept incoming connections
    client_len = sizeof(client);
    new_sock = accept(sockfd, (struct sockaddr*)&client, &client_len);
    if (new_sock == -1) {
        perror("Accept failed");
        close(sockfd);
        return 1;
    }

    // Fork a process for monitoring shared memory
    pid_t pid = fork();

    if (pid == 0) {
        // Child process: Monitor shared memory
        close(sockfd); // Close listening socket in child process
        monitorSharedMemory(shm_ptr, new_sock); // Pass client socket to monitor function
        exit(0); // Exit child process
    } else if (pid > 0) {
        // Parent process: Handle incoming data
        handleClientData(new_sock, shm_ptr);
    } else {
        perror("Fork failed");
        return 1;
    }

    // Detach and delete the shared memory segment
    shmdt(shm_ptr);
    shmctl(shmid, IPC_RMID, NULL);

    // Close sockets
    close(new_sock);
    close(sockfd);

    return 0;
}


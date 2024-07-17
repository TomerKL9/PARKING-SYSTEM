#include "uart.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>

#define PIPE_READ  0
#define PIPE_WRITE 1
#define SOCKETERROR 0
#define FCNTlERROR -1
#define LSEEKERROR -1
#define UARTERROR -1
#define PIPEERROR -1
#define PIDERROR -1
#define RecieveReadERROR -1
#define SendFailed -1
#define IN 0

#define SERVER_IP "192.168.7.20"
#define SERVER_PORT 12345

struct message {
    char number;
    char direction;
};

void parent_process( int pipe_write_fd);
void child_process(int pipe_read_fd);
volatile int CHILED_PIPE_FLAG = 1;
volatile int PERENT_PIPE_FLAG = 1;
/**
 * @brief Main function orchestrating the parent-child process relationship.
 * 
 * This function creates a pipe for communication between a parent and child process.
 * It forks a child process to handle socket communication with a server, while the
 * parent process manages UART communication and controls the operation flow.
 * 
 * @return EXIT_SUCCESS on successful execution, EXIT_FAILURE on failure.
 */
int main() {
    
    int pipe_fds[2];
    if (pipe(pipe_fds) == PIPEERROR) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid == PIDERROR) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) { // Child process
        close(pipe_fds[PIPE_WRITE]); // Close unused write end
        child_process(pipe_fds[PIPE_READ]);
    } else { // Parent process
        close(pipe_fds[PIPE_READ]); // Close unused read end
        parent_process(pipe_fds[PIPE_WRITE]);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Child process function handling socket communication with a server.
 * 
 * This function is executed by the child process. It establishes a connection to
 * a server specified by SERVER_IP and SERVER_PORT using TCP/IP sockets. It reads
 * data from a pipe connected to the parent process and sends it to the server.
 * It also reads data from the server and prints it to stdout.
 * 
 * @param pipe_read_fd File descriptor of the read end of the pipe connected to the parent process.
 */
void child_process(int pipe_read_fd) {
    int sockfd;
    struct sockaddr_in servaddr;
    char buffer[20];
    ssize_t bytesRead;
    char server_message[400];
    int bytes_received;
    int flags;
    
    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < SOCKETERROR) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Fill server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr) <= SOCKETERROR) {
        perror("Invalid address/ Address not supported");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < SOCKETERROR) {
        perror("Connection failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Set sockfd to non-blocking mode
    flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        exit(EXIT_FAILURE);
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == FCNTlERROR) {
        perror("fcntl F_SETFL O_NONBLOCK");
        exit(EXIT_FAILURE);
    }

    // Set pipe_read_fd to non-blocking mode
    flags = fcntl(pipe_read_fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        exit(EXIT_FAILURE);
    }
    if (fcntl(pipe_read_fd, F_SETFL, flags | O_NONBLOCK) == FCNTlERROR) {
        perror("fcntl F_SETFL O_NONBLOCK");
        exit(EXIT_FAILURE);
    }

    while (CHILED_PIPE_FLAG) {
        // Read from the pipe
        bytesRead = read(pipe_read_fd, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0'; // Null terminate the buffer
            //printf("Read from pipe: %d%c\n", buffer[0],buffer[1]);

            // Send data to the server
            if (send(sockfd, buffer, strlen(buffer), 0) == SendFailed) {
                perror("Send failed");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
        } else if (bytesRead == RecieveReadERROR && errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("Pipe read error");
            exit(EXIT_FAILURE);
        }

        // Check for messages from the server
        bytes_received = recv(sockfd, server_message, sizeof(server_message) - 1, 0);
        if (bytes_received > 0) {
            server_message[bytes_received] = '\0';
            printf("Received %s\n", server_message);
        } else if (bytes_received == RecieveReadERROR && errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("Receive from server error");
            exit(EXIT_FAILURE);
        }

        usleep(100000); // Sleep for 100 ms before checking again
    }

    close(sockfd);
}

/**
 * @brief Parent process function managing UART communication and sending data to child process.
 * 
 * This function is executed by the parent process. It initializes UART communication,
 * reads data from the UART, constructs a message, and writes it to a pipe connected
 * to the child process. It also handles user input for controlling the operation.
 * 
 * @param pipe_write_fd File descriptor of the write end of the pipe connected to the child process.
 */
void parent_process( int pipe_write_fd) {
    uart_t myUart;
    char buffer[10];
    struct message message;
    const char *wake = "wake";
    int gpioValue, prev_gpioValue = -1; // Initialize prev_gpioValue
    char input;
    uint8_t inOut = IN;  // Initialize inOut to IN mode

    if (!uart_openDevice(&myUart, "/dev/ttyS4")) {
        fprintf(stderr, "Failed to open port\n");
        exit(EXIT_FAILURE);
    }

    myUart.baudrate = BAUDRATE_115200;
    myUart.parity = 0;
    myUart.dataBits = CS8;
    myUart.stopBits = 0;
    myUart.vmin = 1;
    myUart.timeout = 10;

    if (!uart_initDevice(&myUart)) {
        fprintf(stderr, "Failed to init UART\n");
        uart_closeDevice(&myUart);
        exit(EXIT_FAILURE);
    }
    
    while (PERENT_PIPE_FLAG) {
        printf("Press Enter to continue, 'r' to reset, or 'q' to quit: \n");
        input = getchar(); // Get a single character input
        if (input == '\n') {
            printf("Continuing\n");
        } else if (input == 'q') {
            printf("Quitting\n");
            break;
        } else if (input == 'r') {
            printf("Resetting\n");
            inOut = IN;
        } else {
            // Handle invalid character input
            printf("Invalid character '%c'. Please try again.\n", input);
            // Consume remaining characters in input buffer (until newline or EOF)
            while (getchar() != '\n' && getchar() != EOF);
        }

        if (inOut == IN) { // in
            if (uart_writeData(&myUart, wake, strlen(wake)) == UARTERROR) {
                fprintf(stderr, "Failed to write to UART\n");
                uart_closeDevice(&myUart);
                exit(EXIT_FAILURE);
            }

            if (uart_readData(&myUart, buffer, sizeof(buffer)) == UARTERROR) {
                fprintf(stderr, "Failed to read from UART\n");
                uart_closeDevice(&myUart);
                exit(EXIT_FAILURE);
            }

            // Construct message: received value + 'I'
            message.number = buffer[0]; // Copy received value
            message.direction = 'I'; // Append 'I'
        } else { // out
            // Construct message: received value + 'O'
            message.number = buffer[0]; // Use the last received value
            message.direction = 'O'; // Append 'O'
        }

        inOut = !inOut;
        write(pipe_write_fd, &message, sizeof(message));
    }
    uart_closeDevice(&myUart);
}


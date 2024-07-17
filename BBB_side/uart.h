#ifndef UART_H
#define UART_H

#include <stdbool.h>
#include <stdint.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

typedef enum {
    BAUDRATE_9600 = B9600,
    BAUDRATE_19200 = B19200,
    BAUDRATE_38400 = B38400,
    BAUDRATE_57600 = B57600,
    BAUDRATE_115200 = B115200
} Baudrate;

typedef struct {
    Baudrate baudrate;
    char parity;
    uint32_t dataBits;
    uint32_t stopBits;
    uint32_t vmin;
    uint32_t timeout;
    int fd;
    bool isOpen;
    struct termios uart;
} uart_t;

/**
 * @brief Open the UART device.
 * 
 * This function opens the UART device at the specified path with read/write permissions.
 * 
 * @param uart Pointer to the uart_t structure representing the UART device.
 * @param path Path to the UART device (e.g., "/dev/ttyS0").
 * @return true if the UART device is successfully opened, false otherwise.
 */
bool uart_openDevice(uart_t* uart, const char* path);

/**
 * @brief Close the UART device.
 * 
 * This function closes the UART device associated with the given uart_t structure.
 * 
 * @param uart Pointer to the uart_t structure representing the UART device.
 * @return true if the UART device is successfully closed, false otherwise.
 */
bool uart_closeDevice(uart_t* uart);

/**
 * @brief Initialize the UART device with specified parameters.
 * 
 * This function initializes the UART device with baud rate, parity, stop bits,
 * data bits, timeout, and other settings specified in the uart_t structure.
 * 
 * @param uart Pointer to the uart_t structure representing the UART device.
 * @return true if the UART device is successfully initialized, false otherwise.
 */
bool uart_initDevice(uart_t* uart);

/**
 * @brief Write data to the UART device.
 * 
 * This function writes data to the UART device specified by the uart_t structure.
 * 
 * @param uart Pointer to the uart_t structure representing the UART device.
 * @param buff Pointer to the buffer containing data to be written.
 * @param length Number of bytes to write.
 * @return On success, the number of bytes written is returned. On error, -1 is returned.
 */
ssize_t uart_writeData(uart_t* uart, const char* buff, size_t length);

/**
 * @brief Read data from the UART device.
 * 
 * This function reads data from the UART device specified by the uart_t structure.
 * 
 * @param uart Pointer to the uart_t structure representing the UART device.
 * @param buff Pointer to the buffer where data will be stored.
 * @param sizeRead Maximum number of bytes to read.
 * @return On success, the number of bytes read is returned. On error, -1 is returned.
 */
ssize_t uart_readData(uart_t* uart, char* buff, size_t sizeRead);

/**
 * @brief Check if the UART device is open.
 * 
 * This function checks if the UART device associated with the uart_t structure is open.
 * 
 * @param uart Pointer to the uart_t structure representing the UART device.
 * @return true if the UART device is open, false otherwise.
 */
bool uart_isOpen(uart_t* uart);

#endif // UART_H

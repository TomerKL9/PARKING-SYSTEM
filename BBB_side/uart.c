#include "uart.h"

/**
 * @brief Open the UART device.
 * 
 * This function opens the UART device at the specified path with read/write permissions.
 * 
 * @param uart Pointer to the uart_t structure representing the UART device.
 * @param path Path to the UART device (e.g., "/dev/ttyS0").
 * @return true if the UART device is successfully opened, false otherwise.
 */
bool uart_openDevice(uart_t* uart, const char* path) {
    uart->fd = open(path, O_RDWR | O_NOCTTY);
    if (uart->fd == -1)
        return uart->isOpen;
    uart->isOpen = true;
    return uart->isOpen;
}

/**
 * @brief Close the UART device.
 * 
 * This function closes the UART device associated with the given uart_t structure.
 * 
 * @param uart Pointer to the uart_t structure representing the UART device.
 * @return true if the UART device is successfully closed, false otherwise.
 */
bool uart_closeDevice(uart_t* uart) {
    if (uart_isOpen(uart) && close(uart->fd) == 0) {
        uart->isOpen = false;
        return true;
    }
    return false;
}

/**
 * @brief Initialize the UART device with specified parameters.
 * 
 * This function initializes the UART device with baud rate, parity, stop bits,
 * data bits, timeout, and other settings specified in the uart_t structure.
 * 
 * @param uart Pointer to the uart_t structure representing the UART device.
 * @return true if the UART device is successfully initialized, false otherwise.
 */
bool uart_initDevice(uart_t* uart) {
    if (tcgetattr(uart->fd, &uart->uart) != 0) {
        return false;
    }
    cfsetispeed(&uart->uart, uart->baudrate);
    cfsetospeed(&uart->uart, uart->baudrate);

    uart->uart.c_cflag &= ~PARENB;
    uart->uart.c_cflag &= ~CSTOPB;
    uart->uart.c_cflag &= ~CSIZE;
    uart->uart.c_iflag = IGNBRK;
    uart->uart.c_lflag = 0;
    uart->uart.c_oflag = 0;
    uart->uart.c_cflag |= uart->parity;
    uart->uart.c_cflag |= uart->stopBits;
    uart->uart.c_cflag |= uart->dataBits;
    uart->uart.c_lflag &= ~(IXON | IXOFF | IXANY);
    uart->uart.c_cc[VTIME] = uart->timeout;
    uart->uart.c_cc[VMIN] = uart->vmin;
    uart->uart.c_cflag |= (CLOCAL | CREAD);
    if (tcsetattr(uart->fd, TCSANOW, &uart->uart) != 0) {
        return false;
    }
    return true;
}

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
ssize_t uart_writeData(uart_t* uart, const char* buff, size_t length) {
    return write(uart->fd, buff, length);
}

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
ssize_t uart_readData(uart_t* uart, char* buff, size_t sizeRead) {
    return read(uart->fd, buff, sizeRead);
}

/**
 * @brief Check if the UART device is open.
 * 
 * This function checks if the UART device associated with the uart_t structure is open.
 * 
 * @param uart Pointer to the uart_t structure representing the UART device.
 * @return true if the UART device is open, false otherwise.
 */
bool uart_isOpen(uart_t* uart) {
    return uart->isOpen;
}

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

bool uart_openDevice(uart_t* uart, const char* path);
bool uart_initDevice(uart_t* uart);
bool uart_closeDevice(uart_t* uart);
ssize_t uart_writeData(uart_t* uart, const char* buff, size_t length);
ssize_t uart_readData(uart_t* uart, char* buff, size_t sizeRead);
bool uart_isOpen(uart_t* uart);

#endif // UART_H

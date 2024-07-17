#include "uart.h"


bool uart_openDevice(uart_t* uart, const char* path) {
    uart->fd = open(path, O_RDWR | O_NOCTTY);
    if (uart->fd == -1)
        return uart->isOpen;
    uart->isOpen = true;
    return uart->isOpen;
}


bool uart_closeDevice(uart_t* uart) {
    if (uart_isOpen(uart) && close(uart->fd) == 0) {
        uart->isOpen = false;
        return true;
    }
    return false;
}


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


ssize_t uart_writeData(uart_t* uart, const char* buff, size_t length) {
    return write(uart->fd, buff, length);
}


ssize_t uart_readData(uart_t* uart, char* buff, size_t sizeRead) {
    return read(uart->fd, buff, sizeRead);
}


bool uart_isOpen(uart_t* uart) {
    return uart->isOpen;
}

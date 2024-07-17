#include "gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define GPIO_PATH "/sys/class/gpio"

/**
 * @brief Export a GPIO pin to userspace.
 * 
 * This function exports a GPIO pin to userspace by writing the pin number to the
 * sysfs `export` file.
 * 
 * @param pin The GPIO pin number to be exported.
 */
void gpio_export(int pin) {
    char buffer[64];
    int len = snprintf(buffer, sizeof(buffer), "%d", pin);
    int fd = open(GPIO_PATH "/export", O_WRONLY);
    if (fd < 0) {
        perror("Failed to open export for writing");
        return;
    }
    if (write(fd, buffer, len) < 0) {
        perror("Failed to write to export");
    }
    close(fd);
}

/**
 * @brief Unexport a GPIO pin from userspace.
 * 
 * This function unexports a GPIO pin from userspace by writing the pin number to the
 * sysfs `unexport` file.
 * 
 * @param pin The GPIO pin number to be unexported.
 */
void gpio_unexport(int pin) {
    char buffer[64];
    int len = snprintf(buffer, sizeof(buffer), "%d", pin);
    int fd = open(GPIO_PATH "/unexport", O_WRONLY);
    if (fd < 0) {
        perror("Failed to open unexport for writing");
        return;
    }
    if (write(fd, buffer, len) < 0) {
        perror("Failed to write to unexport");
    }
    close(fd);
}

/**
 * @brief Set the direction of a GPIO pin.
 * 
 * This function sets the direction (input or output) of a GPIO pin by writing the direction
 * string ("in" or "out") to the direction file of the GPIO pin.
 * 
 * @param pin The GPIO pin number whose direction is to be set.
 * @param direction The direction string ("in" for input, "out" for output).
 */
void gpio_set_direction(int pin, const char *direction) {
    char path[64];
    snprintf(path, sizeof(path), GPIO_PATH "/gpio%d/direction", pin);
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open gpio direction for writing");
        return;
    }
    if (write(fd, direction, strlen(direction)) < 0) {
        perror("Failed to write to direction");
    }
    close(fd);
}

/**
 * @brief Read the value of a GPIO pin.
 * 
 * This function reads the current value (0 or 1) of a GPIO pin from the value file
 * of the GPIO pin.
 * 
 * @param pin The GPIO pin number whose value is to be read.
 * @return The current value of the GPIO pin (0 or 1), or -1 if there's an error.
 */
int gpio_get_value(int pin) {
    char path[64];
    char value_str[3];
    snprintf(path, sizeof(path), GPIO_PATH "/gpio%d/value", pin);
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open gpio value for reading");
        return -1;
    }
    if (read(fd, value_str, 3) < 0) {
        perror("Failed to read value");
        close(fd);
        return -1;
    }
    close(fd);
    return atoi(value_str);
}

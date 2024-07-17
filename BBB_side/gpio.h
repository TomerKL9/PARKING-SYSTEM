#ifndef GPIO_H
#define GPIO_H

void gpio_export(int pin);
void gpio_unexport(int pin);
void gpio_set_direction(int pin, const char *direction);
int gpio_get_value(int pin);

#endif // GPIO_H

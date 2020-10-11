#ifndef __GPIOCONTROLLER__
#define __GPIOCONTROLLER__

#include <stdio.h>
#include <bcm2835.h>
#include "types.h"
 
#define WIND RPI_GPIO_P1_18
#define FIRE RPI_GPIO_P1_16
#define BALANCE 0
#define PWM_CHANNEL 1
#define RANGE 1

int on;

int getNextAction(temperature *temperatures);
void handleHardware(int action, int pin);
void gpio_temperatureControl(void *args);
void shutdown();
#endif
#include "gpio_controller.h"

int ventoinhaOn = 0;
int resistorOn = 0;


int getNextAction(temperature* temperatures){
  float desiredTemperature = temperatures->manualControl != 0 ? temperatures->manualControl : temperatures->analogicControl;

  float lowerBound = desiredTemperature - temperatures->histerese / 2;
  float upperBound = desiredTemperature + temperatures->histerese / 2;

  int action = BALANCE;

  if(temperatures->in > upperBound){
    action = WIND;
  }
  else if(temperatures->out < lowerBound){
    action = FIRE;
  }

  on = action;
  return action;
}

void handleHardware(int action, int pin){
  bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
  action == pin ?  bcm2835_gpio_clr(pin) : bcm2835_gpio_set(pin);
	bcm2835_delay(500);
}

void gpio_temperatureControl(void* args)
{
  temperature *temperatures = (temperature*) args;
  int nextAction = getNextAction(temperatures), shouldRun = 1;
  if(nextAction == BALANCE && ventoinhaOn == 0 && resistorOn == 0){
    shouldRun = 0;
  }
  else if(nextAction == WIND && ventoinhaOn == 1 && resistorOn == 0){
    shouldRun = 0;
  }
  else if(nextAction == FIRE && ventoinhaOn == 0 && resistorOn == 1){
    shouldRun = 0;
  }
  ventoinhaOn = nextAction == WIND ? 1 : 0;
  resistorOn = nextAction == FIRE ? 1 : 0;

  if (shouldRun){
      handleHardware(nextAction, WIND);
      handleHardware(nextAction, FIRE);
  }
}

#include "gpio_controller.h"

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

  if (!bcm2835_init())
    return;

  handleHardware(getNextAction(temperatures), WIND);
  handleHardware(getNextAction(temperatures), FIRE);

  bcm2835_close();
}

void shutdown(){
  if (bcm2835_init()){
    handleHardware(BALANCE, WIND);
    handleHardware(BALANCE, FIRE);
    bcm2835_close();
  }
  exit(0);
}

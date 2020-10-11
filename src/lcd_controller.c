#include "lcd_controller.h"

void* lcd_print(void* args)   {
  temperature *temperatures = args;
  if (wiringPiSetup() == -1)
    exit(1);

  fd = wiringPiI2CSetup(I2C_ADDR);

  //printf("fd = %d ", fd);

  lcd_init(); // setup LCD

  char line1[20];
  char line2[20];

  sprintf (line1, "TI %.2f TE %.2f", temperatures->in, temperatures->out);
  sprintf (line2, "TR %.2f", temperatures->manualControl != 0 ? temperatures->manualControl : temperatures->analogicControl);
  ClrLcd();
  lcdLoc(LINE1);
  typeln(line1);
  lcdLoc(LINE2);
  typeln(line2);

  return NULL;
}
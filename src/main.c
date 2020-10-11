#include "includes.h"

/* Variáveis */
temperature temperatures;
pthread_t userInterruptThread;
int printInfoCounter = 0, logCounter = 0, lcdCounter = 0, gpioCounter = 0;

/* "Semafaros" para evitar problema de enviar dado antes da comunicação anterior ter sido finalizada.*/
int outerTemperatureMonitorSemaphore = 0, printDataLocker = 0, forcePrint = 0, inputLocker = 0, arduinoLocker = 0, gpioLocker = 0;

/* Funções */
void alarmHandler();
void *outerTemperatureMonitor(void *args);
void *innerAnalogTemperatureMonitor(void *unused);
void *printTemperatureData(void *unused);
void *handleUserInterrupt(void *unused);
void *temperatureControlThread(void *unused);

int main(int argc, char *argv[])
{
  /* Valor padrão da histerese*/
  temperatures.histerese = 4.f;
  initCSV();

  /* Setando handler do alarm */
  signal(SIGALRM, alarmHandler);
  signal(SIGINT, shutdown);

  /* Iniciando loop de atualização a cada 500ms*/
  ualarm(100000, 100000);

  /* Preparando thread de interrupção do usuário para definir a temperatura */

  while (1)
  {
    sleep(2);
    if(!inputLocker){
      inputLocker = 1;
      pthread_create(&userInterruptThread, NULL, &handleUserInterrupt, NULL);
    }
  }
  return 0;
}

void alarmHandler(){
  pthread_t outerTemperatureMonitorThread, innerAnalogTemperatureMonitorThread, printTemperatureDataThread, lcdThread, saveInFileThread, gpioThread;

  if(!gpioLocker && gpioCounter >= 10){
    gpioLocker = 1;
    gpioCounter = 0;
    pthread_create(&gpioThread, NULL, &temperatureControlThread, (void *)&temperatures);
  }

  if(!arduinoLocker){
    arduinoLocker = 1;
    pthread_create(&innerAnalogTemperatureMonitorThread, NULL, &innerAnalogTemperatureMonitor, NULL);
  }

  if(lcdCounter >= 4){
    lcdCounter = 0;
    pthread_create(&lcdThread, NULL, &lcd_print, (void *)&temperatures);
  }

  if (!outerTemperatureMonitorSemaphore)
  {
    outerTemperatureMonitorSemaphore = 1;
    pthread_create(&outerTemperatureMonitorThread, NULL, &outerTemperatureMonitor, NULL);
  }
  if (printInfoCounter >= 5 || temperatures.manualControl == 0 || forcePrint /*Atualiza a cada 500ms exceto se o potenciometro tiver ligado, nesse caso atualiza a cada 100ms (no minimo)*/)
  {
    forcePrint = 0;
    printInfoCounter = 0;
    if(!printDataLocker)
      pthread_create (&printTemperatureDataThread, NULL, &printTemperatureData, NULL);
  }

  if(logCounter >= 20){
    logCounter = 0;
    pthread_create (&saveInFileThread, NULL, &saveInFile, (void*)&temperatures);
    
  }
  
  gpioCounter++;
  logCounter++;
  printInfoCounter++;
}

void* outerTemperatureMonitor(void* unused){
  BME280_updateTemperature(&temperatures);
  outerTemperatureMonitorSemaphore = 0;
  return NULL;
}

void* innerAnalogTemperatureMonitor(void* unused){
  temperatures.analogicControl = ARD_getAnalogicTemperatureData();
  temperatures.in = ARD_getInnerTemperatureData();
  arduinoLocker = 0;
  return NULL;
}

void* printTemperatureData(void* unused){
  struct tm *data_hora_atual;
  time_t segundos;
  time(&segundos);   
  data_hora_atual = localtime(&segundos);
  system("clear");
  printf("%d:", data_hora_atual->tm_hour); //hora
  printf("%d:",data_hora_atual->tm_min);//minuto
  printf("%d\n\n",data_hora_atual->tm_sec);//segundo  
  printf("Atualmente ligado: %s\n", on == BALANCE ? "Nada" : on == WIND ? "Ventoinha" : "Resistor");
  printf("Temperatura Interna: %.2f\n", temperatures.in);
  printf("Temperatura Externa : % .2f\n", temperatures.out);
  printf("Temperatura Referência (%s): % .2f (histerese: %.2f)\n\n", temperatures.manualControl != 0 ? "Manual" : "Potenciômetro" , temperatures.manualControl != 0 ? temperatures.manualControl : temperatures.analogicControl, temperatures.histerese);
  printf("Pressione Enter para definir a temperatura de referência\n");
  return NULL;
}

void* handleUserInterrupt(void* unused){
  char interrupt, line[20];
  float temp;
  /* Aguardando usuário pressionar enter */
  do{
    interrupt = getchar();
  } while (interrupt != '\n');
  
  printDataLocker = 1;
  system("clear");
  printf("Digite uma temperatura de referência ou uma opção\n-1 - Definir histerese\n0- Usar valor do potenciômetro\n>0 - Definirá a temperatura de referência.\n");
  fgets(line, sizeof(line), stdin);
  sscanf(line, "%f", &temp);
  if(temp >= 0)
    temperatures.manualControl = temp;
  else{
    system("clear");
    printf("Digite um valor para a histerese\n");
    fgets(line, sizeof(line), stdin);
    sscanf(line, "%f", &temperatures.histerese);
  }
  inputLocker = 0;
  printDataLocker = 0;
  forcePrint = 1;
  return NULL;
}

void* temperatureControlThread(void* args){
  gpio_temperatureControl(args);
  gpioLocker = 0;
  return NULL;
}
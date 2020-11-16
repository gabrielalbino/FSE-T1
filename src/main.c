#include "includes.h"

/* Variáveis */
volatile temperature temperatures;
volatile int printInfoCounter = 0, logCounter = 0, lcdCounter = 0, gpioCounter = 0;

/* "Semafaros" para evitar problema de enviar dado antes da comunicação anterior ter sido finalizada.*/
volatile int outerTemperatureMonitorLocker = 0, printDataLocker = 0, forcePrint = 0, inputLocker = 0, arduinoLocker = 0, gpioLocker = 0, lcdLocker = 0, shouldPrintData = 0, fileLocker = 0;

/* Threads */
pthread_t userInterruptThread, outerTemperatureMonitorThread, innerAnalogTemperatureMonitorThread, printTemperatureDataThread, lcdThread, saveInFileThread, gpioThread;

/* Funções */
void alarmHandler();
void *outerTemperatureMonitor(void *args);
void *innerAnalogTemperatureMonitor(void *unused);
void *printTemperatureData(void *unused);
void *handleUserInterrupt(void *unused);
void *temperatureControlThread(void *unused);
void *lcd_print_routine(void *args);
void *file_print(void *args);
void shutdown();

int main(int argc, char *argv[])
{

  /* Iniciando threads*/
  pthread_create(&userInterruptThread, NULL, &handleUserInterrupt, NULL);
  pthread_create(&gpioThread, NULL, &temperatureControlThread, (void *)&temperatures);
  pthread_create(&innerAnalogTemperatureMonitorThread, NULL, &innerAnalogTemperatureMonitor, NULL);
  pthread_create(&lcdThread, NULL, &lcd_print_routine, (void *)&temperatures);
  pthread_create(&outerTemperatureMonitorThread, NULL, &outerTemperatureMonitor, NULL);
  pthread_create (&printTemperatureDataThread, NULL, &printTemperatureData, NULL);
  pthread_create (&saveInFileThread, NULL, &file_print, (void*)&temperatures);

  /* Valor padrão da histerese*/
  temperatures.histerese = 4.f;

  /* Inicia o log */
  initCSV();

  /* Abre a conexão com a GPIO*/
  if(!bcm2835_init()){
    printf("Falha ao iniciar GPIO\n");
    return -3;
  }

  /* Setando handler do alarm */
  signal(SIGALRM, alarmHandler);
  signal(SIGINT, shutdown);

  /* Iniciando loop de atualização a cada 100ms*/
  ualarm(100000, 100000);

  /* Preparando thread de interrupção do usuário para definir a temperatura */

  while (1)
  {
    sleep(1);
  }
  return 0;
}

void alarmHandler(){
  if (!gpioLocker && gpioCounter >= 10)
  {
    gpioLocker = 1;
    gpioCounter = 0;
  }

  if(!arduinoLocker){
    arduinoLocker = 1;
  }

  if(lcdCounter >= 4){
    lcdCounter = 0;
    lcdLocker = 1;
  }

  if (!outerTemperatureMonitorLocker)
  {
    outerTemperatureMonitorLocker = 1;
  }
  if (printInfoCounter >= 5 || temperatures.manualControl == 0 || forcePrint /*Atualiza a cada 500ms exceto se o potenciometro tiver ligado, nesse caso atualiza a cada 100ms (no minimo)*/)
  {
    forcePrint = 0;
    printInfoCounter = 0;
    if(!printDataLocker){
      shouldPrintData = 1;
    }
  }

  if(logCounter >= 20){
    logCounter = 0;
    fileLocker = 1;
  }

  gpioCounter++;
  logCounter++;
  printInfoCounter++;
}

void* outerTemperatureMonitor(void* unused){
  if(BME280_setup() == 0){
    while (1)
    {
      if(outerTemperatureMonitorLocker){
        BME280_updateTemperature(&temperatures);
        outerTemperatureMonitorLocker = 0;
      }
      usleep(1000);
    }
    return NULL;
  }else{
    printf("Erro da bme280: %d\n", BME280_setup());
    exit(-1);
  }
}

void* innerAnalogTemperatureMonitor(void* unused){
  while(1){
    if(arduinoLocker){
      float* temp = ARD_getData();
      temperatures.in = temp[0];
      temperatures.analogicControl = temp[1];
      free(temp);
      arduinoLocker = 0;
    }
    usleep(1000);
  }
  return NULL;
}

void* printTemperatureData(void* unused){
  struct tm *data_hora_atual;
  time_t segundos;
  while(1){
    if(shouldPrintData){
      time(&segundos);   
      data_hora_atual = localtime(&segundos);
//      system("clear");
      printf("%d:", data_hora_atual->tm_hour); //hora
      printf("%d:",data_hora_atual->tm_min);//minuto
      printf("%d\n\n",data_hora_atual->tm_sec);//segundo  
      printf("Atualmente ligado: %s\n", on == BALANCE ? "Nada" : on == WIND ? "Ventoinha" : "Resistor");
      printf("Temperatura Interna: %.2f\n", temperatures.in);
      printf("Temperatura Externa : % .2f\n", temperatures.out);
      printf("Temperatura Referência (%s): % .2f (histerese: %.2f)\n\n", temperatures.manualControl != 0 ? "Manual" : "Potenciômetro" , temperatures.manualControl != 0 ? temperatures.manualControl : temperatures.analogicControl, temperatures.histerese);
      printf("Pressione Enter para definir a temperatura de referência\n");
      shouldPrintData = 0;
    }
  }
  return NULL;
}

void* handleUserInterrupt(void* unused){
  while(1){
    char interrupt, line[20];
    float temp;
    int retry = 0;
    /* Aguardando usuário pressionar enter */
    do{
      interrupt = getchar();
    } while (interrupt != '\n');
    
    printDataLocker = 1;
    do{
      system("clear");
      if(retry){
        printf("O valor deve ser maior que a temperatura externa(%.2fºC)\n\n", temperatures.out);
      }
      printf("Digite uma temperatura de referência ou uma opção\n-1 - Definir histerese\n 0 - Usar valor do potenciômetro\n Número Positivo - Definirá a temperatura de referência.\n");
      fgets(line, sizeof(line), stdin);
      sscanf(line, "%f", &temp);
      retry = 1;
    } while (temp > 0 && temp < temperatures.out);
    if (temp >= 0)
      temperatures.manualControl = temp;
    else if(temp == -1){
      system("clear");
      printf("Digite um valor para a histerese\n");
      fgets(line, sizeof(line), stdin);
      sscanf(line, "%f", &temperatures.histerese);
    }
    inputLocker = 0;
    printDataLocker = 0;
    forcePrint = 1;
    usleep(1000);
  }
  return NULL;
}

void* temperatureControlThread(void* args){
  while(1){
    if(gpioLocker){
      gpio_temperatureControl(args);
      gpioLocker = 0;
    }
    usleep(1000);
  }
  return NULL;
}


void* lcd_print_routine(void* args)   {
  while(1){
    if(lcdLocker){
      lcd_print(args);
      lcdLocker = 0;
    }
    usleep(1000);
  }
  return NULL;
}

void *file_print(void *args){
  while(1){
    if(fileLocker){
      saveInFile(args);
      fileLocker = 0;
    }
    usleep(1000);
  }
}


void shutdown(){
  pthread_cancel(userInterruptThread);
  pthread_cancel(outerTemperatureMonitorThread);
  pthread_cancel(innerAnalogTemperatureMonitorThread);
  pthread_cancel(printTemperatureDataThread);
  pthread_cancel(lcdThread);
  pthread_cancel(saveInFileThread);
  pthread_cancel(gpioThread);
  handleHardware(BALANCE, WIND);
  handleHardware(BALANCE, FIRE);
  bcm2835_close();
  exit(0);
}

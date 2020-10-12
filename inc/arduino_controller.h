#include <stdio.h>
#include <unistd.h>  //Used for UART
#include <fcntl.h>   //Used for UART
#include <termios.h> //Used for UART
#include <string.h>  //Used for UART

#define INT 1
#define FLOAT 2
#define STRING 3

float* ARD_comunicate(unsigned char **mensagem, int size, int returnType);
float* ARD_getData();

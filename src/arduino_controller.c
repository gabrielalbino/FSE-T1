#include <stdio.h>
#include <unistd.h>  //Used for UART
#include <fcntl.h>   //Used for UART
#include <termios.h> //Used for UART
#include <string.h>  //Used for UART
#include "arduino_controller.h"


float ARD_comunicate(unsigned char *mensagem, int size, int returnType)
{
    int uart0_filestream = -1;

    uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY); //Open in non blocking read/write mode
    if (uart0_filestream == -1)
    {
        printf("Erro - Não foi possível iniciar a UART.\n");
    }
    else
    {
        struct termios options;
        tcgetattr(uart0_filestream, &options);
        options.c_cflag = B115200 | CS8 | CLOCAL | CREAD; //<Set baud rate
        options.c_iflag = IGNPAR;
        options.c_oflag = 0;
        options.c_lflag = 0;
        tcflush(uart0_filestream, TCIFLUSH);
        tcsetattr(uart0_filestream, TCSANOW, &options);

        if (uart0_filestream != -1)
        {
            int count = write(uart0_filestream, &mensagem[0], size);
            if (count < 0)
            {
                printf("UART TX error\n");
            }
        }
        usleep(500000);

        //----- CHECK FOR ANY RX BYTES -----
        if (uart0_filestream != -1)
        {
            // Read up to 255 characters from the port if they are there
            unsigned char rx_buffer[256];
            int rx_length = read(uart0_filestream, (void *)rx_buffer, 255); //Filestream, buffer to store in, number of bytes to read (max)
            if (rx_length < 0)
            {
                printf("Erro na leitura.\n"); //An error occured (will occur if there are no bytes)
            }
            else if (rx_length == 0)
            {
                printf("Nenhum dado disponível.\n"); //No data waiting
            }
            else
            {
                //Bytes received
                close(uart0_filestream);
                return *(float *)rx_buffer;
            }
        }
        close(uart0_filestream);
    }
    return 0;
}

float ARD_getAnalogicTemperatureData()
{
    int code = 0xa2;
    int size = 5;
    int returnType = FLOAT;
    unsigned char tx_buffer[20];
    unsigned char *p_tx_buffer;

    p_tx_buffer = &tx_buffer[0];
    *p_tx_buffer++ = code;
    *p_tx_buffer++ = 8;
    *p_tx_buffer++ = 3;
    *p_tx_buffer++ = 6;
    *p_tx_buffer++ = 1;

    return ARD_comunicate(tx_buffer, size, returnType);
}

float ARD_getInnerTemperatureData()
{
    int code = 0xa1;
    int size = 5;
    int returnType = FLOAT;
    unsigned char tx_buffer[20];
    unsigned char *p_tx_buffer;

    p_tx_buffer = &tx_buffer[0];
    *p_tx_buffer++ = code;
    *p_tx_buffer++ = 8;
    *p_tx_buffer++ = 3;
    *p_tx_buffer++ = 6;
    *p_tx_buffer++ = 1;

    return ARD_comunicate(tx_buffer, size, returnType);
}
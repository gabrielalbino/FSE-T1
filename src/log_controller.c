#include "log_controller.h"


void initCSV(){
    FILE* csv = fopen(CSV_PATH, "w+");
    fwrite("Data e hora,temperatura interna,temperatura externa,temperatura definida pelo usuário\n" , 1 , strlen("Data e hora,temperatura interna,temperatura externa,temperatura definida pelo usuário\n") , csv);
    fclose(csv);
}

void saveInFile(void* args){
    temperature* temperatures = args;
    time_t timer;
    char dateTime[26];
    struct tm* tm_info;

    timer = time(NULL);
    tm_info = localtime(&timer);

    strftime(dateTime, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    char linha[200];
    sprintf(linha, "%s,%.2f,%.2f,%.2f\n", dateTime, temperatures->in, temperatures->out, temperatures->manualControl != 0 ? temperatures->manualControl : temperatures->analogicControl);
    FILE* csv = fopen(CSV_PATH, "a+");
    fwrite(linha, 1, strlen(linha), csv);
    fclose(csv);
}

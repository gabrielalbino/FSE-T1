#include <stdio.h>
#include <time.h>
#include <string.h>
#include "types.h"

#define CSV_PATH "log.csv"

void initCSV();
void *saveInFile(void *args);
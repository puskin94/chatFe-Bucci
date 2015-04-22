#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>


/* Prototypes declaration */

bool readUserFile();
bool createLogFile();
void writeToLog(char message[200]);
void timestamp(char * ts);
void buildLog(char message[100], int action);

/* End Declaration */


#endif

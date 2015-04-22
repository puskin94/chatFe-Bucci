#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>


/* Prototypes declaration */

bool readUserFile();
bool createLogFile();
void writeToLog(char message[100]);
void timestamp(char * ts);

/* End Declaration */


#endif

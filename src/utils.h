#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>


/* Prototypes declaration */

bool readUserFile(char *file);
//void writeToLog(char *file, char *message);
bool createLogFile(char *file);
void timestamp(char * ts);

/* End Declaration */


#endif

#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>


/* Dichiarazione dei prototipi */

bool readUserFile();
bool createLogFile();
void writeToLog(char message[200]);
void timestamp(char * ts);
void buildLog(char message[100], int action);

/* Fine della dichiarazione */


#endif

#ifndef THREADDISPATCHER_H
#define THREADDISPATCHER_H

#include <stdbool.h>

void *launchThreadDispatcher();
void writeOnBufferPC(char *msg);
void initStruct();
bool massiveLogout();
bool readFromBufferPC(char **sender, char **receiver, char **msg);

#endif

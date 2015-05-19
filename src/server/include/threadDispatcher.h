#ifndef THREADDISPATCHER_H
#define THREADDISPATCHER_H

void *launchThreadDispatcher();
void writeOnBufferPC(char *msg);
void initStruct();
void readFromBufferPC(char *sender, char *receiver, char *msg);

#endif

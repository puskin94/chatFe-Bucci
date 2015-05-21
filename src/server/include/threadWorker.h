#ifndef LAUNCH_THREAD_WORKER
#define LAUNCH_THREAD_WORKER

void *launchThreadWorker(void *newConn);
void buildMsgForSocket(int success, char **tmpBuff);
void readAndLoadFromSocket(int sock, int len);
void msgForDispatcher(char *sender, char **tmpBuff);


#endif

#ifndef LAUNCH_THREAD_MAIN
#define LAUNCH_THREAD_MAIN

#include <stdbool.h>
#include <signal.h>

extern sig_atomic_t go;

void *launchThreadMain(void *arg);
bool readUserFile();
void sighand(int sig);

#endif

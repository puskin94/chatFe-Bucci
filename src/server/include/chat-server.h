#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <signal.h>

#define K 256

// definizione di variabili globali.
// questo ne consente l'accesso da qualsiasi file esterno incluso

extern char *userFile;
extern char *logFile;
extern sig_atomic_t go;


void sighand(int sig);

#endif

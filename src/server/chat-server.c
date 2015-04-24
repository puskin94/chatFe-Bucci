#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#include "include/chat-server.h"
#include "include/utils.h"
#include "include/hash.h"
#include "include/threadMain.h"


/* TODO LIST

readUserFile: implementare l'inserimento nella lista

*/


char *userFile;
char *logFile;


int main(int argc, char *argv[]) {

    pthread_t threadMain;
    pthread_attr_t attr;

    int retcode;

    // controllo sul necessario numero di parametri
    if (argc < 3) {
        fprintf(stderr,"Wrong param number\n");
        return -1;
    }

    userFile = argv[1];
    logFile = argv[2];


    // Viene creato il thread Main con l'attributo di 'detached'
    // e gli viene imposta l'esecuzione della funzione 'launchThreadMain'

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    if (pthread_create(&threadMain, NULL, launchThreadMain, &attr)!= 0) {
        buildLog("Failed to create threadMain", 1);
        return -5;
    } else {
        printf("[+] ThreadMain is running...\n");
    }

    retcode = pthread_join(&threadMain,NULL);
    if (retcode != 0) {
        printf("thread non terminato");
    } else {
        printf("thread terminato");
    }


    return 0;
}

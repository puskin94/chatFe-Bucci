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

    int pid;

    // controllo sul necessario numero di parametri
    if (argc < 3) {
        fprintf(stderr,"Wrong param number\n");
        return -1;
    }

    userFile = argv[1];
    logFile = argv[2];

    pid = fork();

    // se il processo figlio viene creato correttamente, avvia il threadMain
    if (pid == 0) {
        // Viene creato il thread Main con l'attributo di 'detached'
        // e gli viene imposta l'esecuzione della funzione 'launchThreadMain'

        if (pthread_create(&threadMain, &attr, &launchThreadMain, NULL)!= 0) {
            buildLog("Failed to create threadMain", 1);
            return -5;
        } else {
            printf("[+] ThreadMain is running...\n");
        }

        pthread_join(threadMain, NULL) ;

    } else if (pid < 0) {
        buildLog("[!] Cannot create child process", 1);
    }


    return 0;
}

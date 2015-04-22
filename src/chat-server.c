#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

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

    int pid;

    // controllo sul necessario numero di parametri
    if (argc < 3) {
        fprintf(stderr,"Wrong param number\n");
        return -1;
    }

    userFile = argv[1];
    logFile = argv[2];

    // creazione del figlio
    pid = fork();

    // funzioni del figlio
    if (pid == 0) {
        // avvio il thread Main
        launchThreadMain();

    } else if (pid < 0) {
        fprintf(stderr,"Cannot create child. Quitting...\n");
        return -4;
    }

    return 0;
}

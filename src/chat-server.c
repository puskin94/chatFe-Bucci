#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "utils.h"
#include "common.h"
#include "hash.h"


/* TODO LIST

readUserFile: implementare l'inserimento nella lista


*/


int main(int argc, char *argv[]) {

    int pid, readFile, writeToLogExit;

    if (argc < 3) {
        fprintf(stderr,"Wrong param number\n");
        return -1;
    }

    char *userFile = argv[1];
    char *logFile = argv[2];

    pid = fork();

    // funzioni del figlio
    if (pid == 0) {
        // read user-file
        if (readUserFile(userFile)) {
            printf("[+] SERVER INFO: loaded user-file\n");
        } else {
            printf("[!] SERVER INFO: cannot load user-file\n");
            return -2;
        }

        // write first message to log-file
        if (createLogFile(logFile)) {
            printf("[+] SERVER INFO: loaded log-file\n");
        } else {
            printf("[!] SERVER INFO: cannot load log-file\n");
            return -3;
        }

    } else if (pid < 0) {
        fprintf(stderr,"Cannot create child. Quitting...\n");
        return -4;
    }

    return 0;
}

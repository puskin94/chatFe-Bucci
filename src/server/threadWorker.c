#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#include "include/chat-server.h"
#include "include/utils.h"
#include "include/hash.h"

#define MSG_LOGIN "L"
#define MSG_REGLOG "R"
#define MSG_OK "O"
#define MSG_ERROR "E"
#define MSG_SINGLE "S"
#define MSG_BRDCAST "B"
#define MSG_LIST "I"
#define MSG_LOGOUT "X"

void *launchThreadWorker(void *newConn) {

    pthread_mutex_t mux = PTHREAD_MUTEX_INITIALIZER;

    char cmdReceived[100];
    char *action;

    char *userName;
    char *fullName;
    char *mail;

    int sock = *(int*)newConn;

    // Viene ricevuto il messaggio e controllato quale servizio
    // viene richiesto

    if(read(sock, cmdReceived, sizeof(cmdReceived)) < 0) {

        printf("[!] Error while reading from socket\n");

    } else {

        action = strtok(cmdReceived, ":");

        if (strcmp(action, MSG_LOGIN) == 0) {
                        // viene richesto il LOGIN
            userName = strtok(NULL, " ");
                        // ricerca di 'username' nella hashtable
                        printf("%s\n",userName);

        } else if (strcmp(action, MSG_REGLOG) == 0) {
                        // viene richiesta la registrazione ed il login
            fullName = strtok(NULL, ":");
            mail = strtok(NULL, ":");
            userName = strtok(NULL, ":");

            printf("%s --> %s --> %s\n",fullName, mail, userName );

        } else if (strcmp(action, MSG_LIST) == 0) {
                        // viene richiesta la lista degli utenti online

        } else if (strcmp(action, MSG_LOGOUT) == 0) {
                        // viene richiesta la registrazione ed il login
        }
    }

    return NULL;
}

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
#include "include/threadWorker.h"

#define PORT 7778

bool readUserFile();
void sighand(int sig);


bool go = true;
int sockId;


void *launchThreadMain(void *arg) {

    int sockId, len, retval, newConn;


    struct sockaddr_in server;
    struct sockaddr_in client;

    // generazione del threadWorker
    pthread_t threadWorker;
    pthread_attr_t attr;

    // aggiunta dell'attributo 'detached'
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    signal(SIGINT, sighand);
    signal(SIGTERM, sighand);


    // read user-file
    if (readUserFile()) {
        printf("[+] SERVER INFO: loaded user-file\n");
    } else {
        printf("[!] SERVER INFO: cannot load user-file\n");
        retval = -2;
        pthread_exit(&retval);
    }

    // write first message to log-file
    if (createLogFile()) {
        printf("[+] SERVER INFO: loaded log-file\n");
    } else {
        printf("[!] SERVER INFO: cannot load log-file\n");
        retval = -3;
        pthread_exit(&retval);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    sockId = socket(AF_INET, SOCK_STREAM, 0);

    if (bind(sockId, (struct sockaddr *)&server, sizeof(server)) < 0) {
        buildLog("Cannot bind on port",1);
    } else {

        listen(sockId, SOMAXCONN);

        len = sizeof(struct sockaddr_in);

        while (go) {
            newConn = accept(sockId, (struct sockaddr *)&client, (socklen_t *)&len);

            if(newConn == -1) {
                buildLog("[!] Cannot accept new connections", 1);
            } else {
                printf("\n[+] New Client Connected\n");

                if(pthread_create(&threadWorker, &attr, &launchThreadWorker, (void *)&newConn) != 0) {
                    buildLog("Failed to create threadWorker", 1);
                }
            }
        }
        close(newConn);

    }
    close(sockId);
    return NULL;

}


/* la funzione sottostante consente di caricare tutti i dati necessari
alla gestione degli utenti nella tabella hash */

bool readUserFile() {

    static hash_t HASH_TABLE;

    FILE *fp;
    char userInfo[771]; // 768 == (256 * 3) + 2 + 1 --> (single max lenght + 2 (:) + 1 (\0))

    char *userName;
    char *fullName;
    char *mail;

    fp = fopen(userFile, "r+");
    if (fp != NULL) {

        HASH_TABLE = CREAHASH();
        hdata_t *user = (hdata_t *) malloc(sizeof(hdata_t));

        while (fgets (userInfo, sizeof(userInfo), fp)) {
            userName = strtok(userInfo, ":");
            fullName = strtok(NULL, ":");
            mail = strtok(NULL, ":\n");

            if (userName != NULL && fullName != NULL && mail != NULL) {

                user->uname = userName;
                user->fullname = fullName;
                user->email = mail;
                user->sockid = -1;

                INSERISCIHASH(user->uname, (void*) user, HASH_TABLE);
            }
        }
    } else {
        buildLog("Cannot load userFile. Quitting...", 1);
        return false;
    }

    fclose(fp);
    return true;

}

void sighand(int sig) {
    if ( sig == SIGINT || sig == SIGTERM ) {
        buildLog("CTRL-C Received. Quitting", 1);
        go = false;
    }
}

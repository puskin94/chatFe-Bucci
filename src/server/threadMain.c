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
#include "include/userManagement.h"
#include "include/threadMain.h"
#include "include/threadDispatcher.h"

#define PORT 7778


sig_atomic_t go;


int sockId;

void *launchThreadMain(void *arg) {

    int sockId, len, retval, newConn;

    int numThreadAttivi = 0;


    struct sockaddr_in server;
    struct sockaddr_in client;

    go = true;

    // generazione del threadWorker
    pthread_t threadWorker;
    pthread_t threadDispatcher;
    pthread_attr_t attr;

    // aggiunta dell'attributo 'detached'
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);


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
        buildLog("Cannot bind on port", 1);
    } else {

        listen(sockId, SOMAXCONN);

        len = sizeof(struct sockaddr_in);

        // creo il thread dispatcher
        if(pthread_create(&threadDispatcher, NULL, &launchThreadDispatcher, NULL) != 0) {
            buildLog("Failed to create threadDispatcher", 1);
        }


        while (go) {
            newConn = accept(sockId, (struct sockaddr *)&client, (socklen_t *)&len);
            if(newConn == -1) {
                buildLog("[!] Cannot accept new connections", 1);
            } else {

                if(pthread_create(&threadWorker, &attr, &launchThreadWorker, (void *)&newConn) != 0) {
                    buildLog("Failed to create threadWorker", 1);
                } else {
                    numThreadAttivi++;
                }
            }
        }

        pthread_join(threadDispatcher, NULL);
        close(newConn);
    }
    close(sockId);
    pthread_exit(NULL);
}

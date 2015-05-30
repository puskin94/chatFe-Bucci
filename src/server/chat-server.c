#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>

#include "include/chat-server.h"
#include "include/utils.h"
#include "include/hash.h"
#include "include/threadMain.h"
#include "include/threadDispatcher.h"
#include "include/userManagement.h"

#define PORT 7778
#define P_LOGOUT "00000O"

// variabile di tipo sig_atomic , necessaria per il signal handler
// è di tipo "atomico" perchè stiamo lavorando con i thread
sig_atomic_t go;

char *userFile;
char *logFile;


void sighand(int sig);


int main(int argc, char *argv[]) {

    pthread_t threadMain;

    go = true;
    int pid;

    struct sigaction sigHandling;
    sigHandling.sa_handler = sighand;

    sigaction(SIGINT, &sigHandling, 0);
    sigaction(SIGTERM, &sigHandling, 0);


    // controllo sul necessario numero di parametri
    if (argc != 3) {
        fprintf(stderr,"Wrong param number\n");
        return -1;
    }

    userFile = argv[1];
    logFile = argv[2];

    pid = fork();

    // se il processo figlio viene creato correttamente, avvia il threadMain
    if (pid == 0) {
        // Viene creato il thread Main
        // e gli viene imposta l'esecuzione della funzione 'launchThreadMain'

        if (pthread_create(&threadMain, NULL, &launchThreadMain, NULL)!= 0) {
            buildLog("Failed to create threadMain", 1);
            return -5;
        }

        pthread_join(threadMain, NULL);

    } else if (pid < 0) {
        buildLog("[!] Cannot create child process", 1);
    }

    return 0;
}

void sighand(int sig) {

    int sockId, receiverId, numThreadAttivi;
    char *msg, *userName, *receiver, *tmpBuff, *logMsg;
    struct sockaddr_in closeConn;

    hdata_t *hashUser = (hdata_t *) malloc(sizeof(struct msg_t*));

    tmpBuff = malloc(sizeof(char));
    receiver = malloc(sizeof(char));
    userName = malloc(sizeof(char));



    if ( sig == SIGINT || sig == SIGTERM ) {
        go = false;

        // saveTable appende al file degli utenti, i dati degli utenti registrati
        // durante la sessione del server
        saveTable();

        /* per 'saziare' i vari socket ancora attivi in attesa, viene creata una
        connessione fasulla. Invece per far disconnettere tutti i client una volta
        che si è deciso che il server vada spento, viene inviato in broadcast
        a tutti gli utenti connessi un messaggio speciale che li invita
        (non troppo gentilmente) a fare il logout*/
        closeConn.sin_family = AF_INET;
        closeConn.sin_port = htons(PORT);
        closeConn.sin_addr.s_addr = INADDR_ANY;

        sockId = socket(AF_INET, SOCK_STREAM, 0);


        connect(sockId, (struct sockaddr *)&closeConn, sizeof(closeConn));

        while (numThreadAttivi != 0) {

            listUser(&tmpBuff);
            receiver = strndup(tmpBuff + 6, strlen(tmpBuff) - 6);

            userName = strtok(receiver, ":");
            msg = strdup(P_LOGOUT);


            do {
                receiverId = returnSockId(userName, hashUser);

                if(send(receiverId , msg , strlen(msg), 0) < 0) {
                    logMsg = strdup("[!] Cannot send Shutdown message to the clients");
                    buildLog(logMsg, 1);
                } else {
                    numThreadAttivi--;
                }

                userName = strtok(NULL, ":");
            } while (userName != NULL);
        }

        close(sockId);

    }

    free(msg); free(userName); free(receiver); free(tmpBuff); free(logMsg);
}

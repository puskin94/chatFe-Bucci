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

// variabile di tipo sig_atomic , necessaria per il signal handler
// è di tipo "atomico" perchè stiamo lavorando con i thread. Alcuni segnali potrebbero
// sovrapporsi portando ad una variabile "impura"
sig_atomic_t go;

char *userFile;
char *logFile;



int main(int argc, char *argv[]) {

    pthread_t threadMain;

    go = true;
    int pid;

    // questa è una struttura che contiene dati relativi al signalHandling
    struct sigaction sigHandling;
    sigHandling.sa_handler = sighand;

    sigaction(SIGINT, &sigHandling, 0);
    sigaction(SIGTERM, &sigHandling, 0);


    // controllo sul necessario numero di parametri
    if (argc != 3) {
        fprintf(stderr,"Wrong param number\n");
        return -1;
    }

    // le variabili "extern" definite nei file .h vengono riempite con i dati
    // passati da linea di comando
    userFile = argv[1];
    logFile = argv[2];

    // raccolgo il risultato della fork per i successivi controlli
    pid = fork();

    // se il processo figlio viene creato correttamente, avvia il threadMain
    if (pid == 0) {
        // Viene creato il thread Main
        // e gli viene imposta l'esecuzione della funzione 'launchThreadMain'

        // se il thread non viene creato correttamente scrivo sul log file
        if (pthread_create(&threadMain, NULL, &launchThreadMain, NULL)!= 0) {
            buildLog("Failed to create threadMain", 1);
            return -5;
        }

        // aspettiamo il threadMain
        pthread_join(threadMain, NULL);

    } else if (pid < 0) {
        buildLog("[!] Cannot create child process", 1);
    }

    return 0;
}

void sighand(int sig) {

    pthread_mutex_t mux = PTHREAD_MUTEX_INITIALIZER;


    if ( sig == SIGINT || sig == SIGTERM ) {

        int sockId;
        struct sockaddr_in closeConn;

        go = false;

        // saveTable appende al file degli utenti, i dati degli utenti registrati
        // durante la sessione del server
        saveTable();

        // per 'saziare' i vari socket ancora attivi in attesa, viene creata una
        // connessione fasulla.
        closeConn.sin_family = AF_INET;
        closeConn.sin_port = htons(PORT);
        closeConn.sin_addr.s_addr = INADDR_ANY;

        sockId = socket(AF_INET, SOCK_STREAM, 0);

        connect(sockId, (struct sockaddr *)&closeConn, sizeof(closeConn));
        close(sockId);

        // alla funzione "writeOnBufferPC" il compito di mandare ai client
        // il comando di disconnessione
        pthread_mutex_lock(&mux);
        writeOnBufferPC("B00000000O");
        pthread_mutex_unlock(&mux);
    }

}

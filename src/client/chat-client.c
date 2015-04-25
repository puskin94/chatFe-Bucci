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

#include "utils.h"

#define PORT 7777
#define MAXCONNTENT 10
#define SERVERIP "127.0.0.1"

int main(int argc, char *argv[]) {

    struct sockaddr_in client;

    // intero che indica il numero di tentativi di connessione al server
    int count = 0, sockId;

    client.sin_family = AF_INET;
    client.sin_port = htons(PORT);
    client.sin_addr.s_addr = inet_addr(SERVERIP); // ip del server

    sockId = socket(AF_INET, SOCK_STREAM, 0);

    // qua viene gestito il caso con il minor numero di parametri:
    // chat-client <username>
    if (argc < 2) {
        fprintf(stderr,"Wrong param number\n");
        return -1;
    }

    // Queste sono le varie casistiche ammesse

    if ( (strcmp(argv[1], "-h") == 0) ||
        ((strcmp(argv[1], "-r") == 0) && argc == 6) ||
        (argc == 2) ) {

        // Il client prova a connettersi al server remoto per un massimo di
        // MAXCONNTENT volte

        while (connect(sockId, (struct sockaddr *)&client, sizeof(client)) < 0 && count < MAXCONNTENT) {
            printf("Server unreachable. Attempt n.%d\n", count+1);
            sleep(1);
            count++;
        }

        if (count == MAXCONNTENT) {
            printf("Cannot connect to the server\n");
            return -1;
        } else {
            printf("Connected to the Server!\n");
        }

        // visualizzazione dell' help
        if (strcmp(argv[1], "-h") == 0) {
            printHelp();
        } else if ((strcmp(argv[1], "-r") == 0) && argc == 6) {
        // se il parametro è '-r' e ci sono tutti i parametri necessari
            printf("Mi devo registrare");

        // Qua ci va la registrazione


        } else if (argc == 2) {
        // ultimo caso: se è presente un solo parametro, deve essere per forza il login
            printf("Sto facendo il login");

        // Qua ci va il login
        }


    }


    return 0;
}

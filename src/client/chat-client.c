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

#include "include/utils.h"

#define PORT 7777
#define MAXCONNTENT 10
#define SERVERIP "127.0.0.1"


#define MSG_LOGIN "L"
#define MSG_REGLOG "R"
#define MSG_OK "O"
#define MSG_ERROR "E"
#define MSG_SINGLE "S"
#define MSG_BRDCAST "B"
#define MSG_LIST "I"
#define MSG_LOGOUT "X"

int main(int argc, char *argv[]) {

    struct sockaddr_in client;

    // intero che indica il numero di tentativi di connessione al server
    int count = 0, sockId;
    char cmdToSend[100];
    char fullName[256];

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
            strcat(cmdToSend, MSG_REGLOG);
            strcat(cmdToSend, ":");
            strcat(cmdToSend, argv[2]); // nome
            strcat(cmdToSend, " ");
            strcat(cmdToSend, argv[3]); // cognome
            strcat(cmdToSend, ":");
            strcat(cmdToSend, argv[4]); // mail
            strcat(cmdToSend, ":");
            strcat(cmdToSend, argv[5]); // username

            if(send(sockId , cmdToSend , sizeof(cmdToSend) , 0) < 0) {
                printf("[!] Cannot send registration request to the server!\n");
            } else {
                printf("[+] Sent Registration Request\n");
            }

        } else if (argc == 2) {
        // ultimo caso: se è presente un solo parametro, deve essere per forza il login
            strcat(cmdToSend, MSG_LOGIN);
            strcat(cmdToSend, ":");
            strcat(cmdToSend, argv[1]);

            if(send(sockId , cmdToSend , sizeof(cmdToSend) , 0) < 0) {
                printf("[!] Cannot send login request to the server!\n");
            } else {
                printf("[+] Sent Login Request\n");
            }

        }


    }


    return 0;
}

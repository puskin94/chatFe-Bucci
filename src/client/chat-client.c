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

#define PORT 7778
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


// questa funzione consente la creazione di una stringa contenente la lunghezza
// in byte del campo passato come parametro
void getLen(char *buff, int len, int threeOrFive) {

    char intToChar[5];
    int i;

    sprintf(intToChar, "%d", len);

    threeOrFive -= strlen(intToChar);
    for (i = 0; i < threeOrFive; i++) {
        strcat(buff, "0");
    }
    strcat(buff, intToChar);

}

int main(int argc, char *argv[]) {

    struct sockaddr_in client;

    // intero che indica il numero di tentativi di connessione al server
    int count = 0, sockId;
    int buffSize = 8;
    int remaining = 5;
    int lenMsg;
    int tmpLenMsg = 8;
    char intToChar[5];

    char name[128];
    char surname[128];
    char mail[256];

    char *tmpMsg = malloc(sizeof(char));
    char *buff = malloc(sizeof(char)); // la dimensione iniziale è quella di un char
                                        // ovvero il primo token da leggere

    //char *buffRisp = malloc(sizeof(char));

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
        ((strcmp(argv[1], "-r") == 0) && argc == 4) ||
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
        } else if ((strcmp(argv[1], "-r") == 0) && argc == 4) {
            // se il parametro è '-r' e ci sono tutti i parametri necessari

            // ./chat-client -r "Giovanni Bucci giovanni01.bucci@student.unife.it" puskin


            strcat(buff,MSG_REGLOG);

            buffSize += 6;
            buff = realloc(buff, buffSize);
            strcat(buff, "000000"); // non ci sono ne sender ne receiver
            buffSize += 5; buff = realloc(buff, buffSize);

            // costruzione del messaggio da inviare

            tmpLenMsg += strlen(argv[3]); tmpMsg = realloc(tmpMsg, tmpLenMsg);
            strcat(tmpMsg, argv[3]); // username

            tmpLenMsg += strlen(":"); tmpMsg = realloc(tmpMsg, tmpLenMsg);
            strcat(tmpMsg, ":");



            if (sscanf(argv[2], "%s %s %s", name, surname, mail ) == 3) {


                tmpLenMsg += strlen(name);
                tmpMsg = realloc(tmpMsg, tmpLenMsg);
                strcat(tmpMsg, name); // nome


                tmpLenMsg += strlen(" "); tmpMsg = realloc(tmpMsg, tmpLenMsg);
                strcat(tmpMsg, " ");


                tmpLenMsg += strlen(surname); tmpMsg = realloc(tmpMsg, tmpLenMsg);
                strcat(tmpMsg, surname); // cognome

                tmpLenMsg += strlen(":"); tmpMsg = realloc(tmpMsg, tmpLenMsg);
                strcat(tmpMsg, ":");


                tmpLenMsg += strlen(mail); tmpMsg = realloc(tmpMsg, tmpLenMsg);
                strcat(tmpMsg, mail); // mail

            }





            lenMsg = strlen(tmpMsg);
            getLen(buff, lenMsg, 5); // calcolo la lunghezza del messaggio successivo

            buffSize += lenMsg; buff = realloc(buff, buffSize);
            strcat(buff, tmpMsg);



            if(send(sockId , buff , buffSize , 0) < 0) {
                printf("[!] Cannot send registration request to the server!\n");
            } else {
                printf("[+] Sent Registration Request\n");
            }

        } else if (argc == 2) {
            // ultimo caso: se è presente un solo parametro, deve essere per forza il login
            strcat(buff, MSG_LOGIN);

            buffSize += 6;
            buff = realloc(buff, buffSize);
            strcat(buff, "000000"); // non ci sono ne sender ne receiver
            buffSize += 5; buff = realloc(buff, buffSize);

            lenMsg = strlen(argv[1]);
            getLen(buff, lenMsg, 5); // calcolo la lunghezza del messaggio successivo

            buffSize += lenMsg; buff = realloc(buff, buffSize);
            strcat(buff, argv[1]);


            if(send(sockId , buff , buffSize , 0) < 0) {
                printf("[!] Cannot send login request to the server!\n");
            } else {
                printf("[+] Sent Login Request\n");
            }
        }

        // qualunque sia il comando inviato, devo aspettare un messaggio
        // di risposta ( MSG_OK || MSG_ERROR ) dal server

/*        if(read(sockId, buffRisp, 3 < 0)) {
            printf("non ricevo nulla\n");
        } else {
            if (strcmp(buffRisp, "OK") == 0) {
                printf("molto bene\n");
            } else {
                printf("molto male\n");
            }
            printf("%s\n", buffRisp);
        }*/


    }


    return 0;
}

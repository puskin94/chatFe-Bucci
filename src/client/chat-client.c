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
#include "include/threadReader.h"

#define PORT 7778
#define MAXCONNTENT 10
#define SERVERIP "127.0.0.1"


#define MSG_LOGIN 'L'
#define MSG_REGLOG 'R'
#define MSG_OK 'O'
#define MSG_ERROR 'E'
#define MSG_SINGLE 'S'
#define MSG_BRDCAST 'B'
#define MSG_LIST 'I'
#define MSG_LOGOUT 'X'



int main(int argc, char *argv[]) {

    struct sockaddr_in client;

    // intero che indica il numero di tentativi di connessione al server
    int count = 0, sockId;
    int buffSize;
    int remaining = 5;
    int lenMsg;
    int tmpLenMsg = 0;
    char intToChar[5];

    char name[128];
    char surname[128];
    char mail[256];

    char *tmpMsg;
    char *buff = malloc(sizeof(char)); // la dimensione iniziale è quella di un char
                                        // ovvero il primo token da leggere

    char *buffRisp = malloc(sizeof(char));

    // Questa variabile booleana è essenziale per capire se qualsiasi richiesta
    // del client ( registrazione || login ) è stata accettata con successo o meno
    bool isIn = false;


    client.sin_family = AF_INET;
    client.sin_port = htons(PORT);
    client.sin_addr.s_addr = inet_addr(SERVERIP); // ip del server

    sockId = socket(AF_INET, SOCK_STREAM, 0);

    pthread_t threadReader;
    pthread_t threadListener;

    // qua viene gestito il caso con il minor numero di parametri:
    // chat-client <username>
    if (argc < 2) {
        fprintf(stderr,"Wrong param number\n");
        return -1;
    }

    if (strcmp(argv[1], "-h") == 0) {
        printHelp();
        return 1;
    }

    // Queste sono le varie casistiche ammesse

    if ( ((strcmp(argv[1], "-r") == 0) && argc == 4) || (argc == 2)) {

        // Il client prova a connettersi al server remoto per un massimo di
        // MAXCONNTENT volte

        while (connect(sockId, (struct sockaddr *)&client, sizeof(client)) < 0 && count < MAXCONNTENT) {
            printf("Server unreachable. Attempt n.%d\n", count+1);
            sleep(1);
            count++;
        }

        if (count == MAXCONNTENT) {
            printf("[!] Cannot connect to the server\n");
            return -1;
        }
        printf("[+] Connected to the Server!\n");


        if ((strcmp(argv[1], "-r") == 0) && argc == 4) {
            // se il parametro è '-r' e ci sono tutti i parametri necessari

            if (sscanf(argv[2], "%s %s %s", name, surname, mail ) == 3) {

                buffSize = 21+strlen(argv[3])+strlen(name)+strlen(surname)+strlen(mail);
                /*
                21 ==
                    6 = dimensione dell'intero messaggio
                    1 = type
                    6 = 000000
                    5 = msglen
                    3 = 2 * : + " "
                */
                buff = realloc(buff, buffSize);

                sprintf(buff,"%06d%c000000%05zu%s:%s %s:%s", buffSize, MSG_REGLOG,
                                                    (strlen(argv[3])+strlen(name)+strlen(surname)+strlen(mail)) + 3,
                                                    argv[3], name, surname, mail);

            }


            if(send(sockId , buff , buffSize , 0) < 0) {
                printf("[!] Cannot send registration request to the server!\n");
            } else {
                printf("[+] Sent Registration Request\n");
            }

        } else if (argc == 2) {
            // ultimo caso: se è presente un solo parametro, deve essere per forza il login
            lenMsg = strlen(argv[1]);
            buffSize = 18 + lenMsg;
            /*
            18 ==
                6 = dimensione dell'intero messaggio
                1 = type
                6 = 000000
                5 = msglen
            */
            buff = realloc(buff, buffSize);

            sprintf(buff,"%06d%c000000%05d%s", buffSize, MSG_LOGIN, lenMsg, argv[1]);

            if(send(sockId , buff , buffSize , 0) < 0) {
                printf("[!] Cannot send login request to the server!\n");
            } else {
                printf("[+] Sent Login Request\n");
            }
        }

        // qualunque sia il comando inviato, devo aspettare un messaggio
        // di risposta ( MSG_OK || MSG_ERROR ) dal server

        if(read(sockId, buffRisp, sizeof(char) )< 0) {
            printf("[!] Error contacting the server\n");
        } else {
            if (buffRisp[0] == MSG_ERROR) {
                // leggo la lunghezza del messaggio di errore
                buffRisp = realloc(buffRisp, 3); // adesso può contenere 3 decimali
                read(sockId, buffRisp, 3);
                lenMsg = sizeof(char) * atoi(buffRisp);

                // rialloco la dimensione del buffer di conseguenza
                buffRisp = realloc(buffRisp, lenMsg);
                // ora sono pronto a leggere la risposta
                read(sockId, buffRisp, lenMsg);
                printf("[!] %s\n", buffRisp);
            } else {
                printf("[+] Action completed successfully!\n");
                isIn = true;
            }
        }

    }

    // questo if viene eseguito solo se il server ha spedito un messaggio di tipo
    // MSG_OK
    if(isIn) {

        // vengono creati 2 thread ( come da consegna ): threadReader & threadListener
        if (pthread_create(&threadReader, NULL, &launchThreadReader,(void *)&sockId)!= 0) {
            fprintf(stderr,"Failed to create threadReader");
            return -2;
        }
        /*if (pthread_create(&threadListener, NULL, &launchThreadListener, (void *)&sockId)!= 0) {
            buildLog("Failed to create threadListener", 1);
            return -3;
        }*/

        pthread_join(threadReader, NULL);
        //pthread_join(threadListener, NULL);

    }
    return 0;
}

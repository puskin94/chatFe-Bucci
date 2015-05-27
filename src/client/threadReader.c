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

#define MSG_LOGIN 'L'
#define MSG_REGLOG 'R'
#define MSG_OK 'O'
#define MSG_ERROR 'E'
#define MSG_SINGLE 'S'
#define MSG_BRDCAST 'B'
#define MSG_LIST 'I'
#define MSG_LOGOUT 'X'


void *launchThreadReader(void *newConn) {

    int sock = *(int*)newConn;
    int numChars;

    char *msgToSend = malloc(sizeof(char));
    char *cmd, *msgText, *msgTo, *strCpy, *msg = NULL;

    bool loggedOut = false;

    size_t msgLen = 0;


    /* la dimensione del messaggio viene allocata dinamicamente con la funzione getline.
        nella versione precedente del code, la dimensione era incrementata
        ogni volta che veniva premuto un tasto. Questo rendeva la quantità
        di realloc veramente alta ammazzando le performances del client.
    */
    while(!loggedOut && (getline(&msg, &msgLen, stdin) != -1 )) {

        msg[strcspn(msg, "\n")] = 0;

        if (msg[0] == '#') {
            /* se si tratta di un messaggio '#dest'
            l'interno di questo costrutto costruisce il messaggio e lo spedisce al
            server. A lui il compito di smistarlo al destinatario */
            if (strncmp(msg, "#dest", 5) == 0) {
                // strtok partiziona la stringa... meglio farne una copia
                strCpy = strdup(msg);

                cmd = strtok(msg, ":");
                msgText = strdup(strtok(NULL, "\n"));

                // se il messaggio è broadcast
                if (strCpy[6] == ':') {
                    numChars = (18 + sizeof(msgText));
                    msgToSend = realloc(msgToSend, numChars * sizeof(char));

                    sprintf(msgToSend, "%06d%c000000%05zu%s", numChars-6,
                        MSG_BRDCAST, strlen(msgText), msgText);

                } else {
                    // se il messaggio è privato
                    msgTo = strdup(strtok(cmd, " ")); msgTo = strdup(strtok(NULL, " "));
                    numChars = (18 + sizeof(msgTo) + sizeof(msgText));
                    msgToSend = realloc(msgToSend, numChars * sizeof(char));

                    sprintf(msgToSend, "%06d%c000%03zu%s%05zu%s", numChars-6,
                        MSG_SINGLE, strlen(msgTo), msgTo, strlen(msgText), msgText);

                }
            } else if (strncmp(msg, "#logout", 7) == 0) {

                numChars = 18;
                msgToSend = realloc(msgToSend, numChars * sizeof(char));
                sprintf(msgToSend, "%06d%c00000000000", numChars-6, MSG_LOGOUT);
                loggedOut = true;

            } else if (strncmp(msg, "#ls", 3) == 0) {

                numChars = 18;
                msgToSend = realloc(msgToSend, numChars * sizeof(char));
                sprintf(msgToSend, "%06d%c00000000000", numChars-6, MSG_LIST);

            }


            if(send(sock , msgToSend , numChars , 0) < 0) {
                fprintf(stderr,"Cannot send the message to the server\n");
            }

        }

    }

    free(msg); free(msgToSend);
    close(sock);
    pthread_exit(NULL);
}

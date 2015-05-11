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
    char *msgToSend = malloc(sizeof(char));
    char *msg = malloc(sizeof(char));
    char *cmd;
    char *msgTo;
    char *msgText;
    char ch;
    int numChars;

    do {
        // il buffer da inviare viene allocato dinamicamente ogni volta
        // che viene premuto un tasto sulla tastiera: Questo metodo ammazza
        // letteralmente le performances del singolo thread ma il numero
        // di bytes inviati al server ne beneficia.
        numChars = 1;
        bzero(msg, sizeof(char) * numChars);

        while((ch = getchar()) != '\n') {
            numChars++;
            if (numChars % 10 == 0) {
                msg = realloc(msg, sizeof(char) * numChars + 10);
            }
            strncat(msg, &ch, 1);
        }

        if (msg[0] == '#') {
            // se si tratta di un messaggio '#dest'
            if (strncmp(msg, "#dest", 5) == 0) {

                cmd = strtok(msg, ":");
                msgText = strdup(strtok(NULL, ":"));
                // faccio la distinzione tra privato e broadcast
                if (cmd[5] == ' ') {
                    msgTo = strdup(strtok(cmd, " ")); strdup(msgTo = strtok(NULL, " "));
                    numChars = (12 + strlen(msgTo) + strlen(msgText));
                    msgToSend = realloc(msgToSend, numChars * sizeof(char));
                    sprintf(msgToSend, "%c000%03zu%s%05zu%s", MSG_SINGLE, strlen(msgTo), msgTo, strlen(msgText), msgText);
                } else {
                    printf("figata, broadcast\n");
                }
            }
/*            cmd = strtok(msg, ":");
            msgText = strtok(NULL, ":");
            printf("msg----->%s\n", msgText);
            // sono presenti i : sse il messaggio è di tipo '#dest'
            // quindi l' if statement successivo viene eseguito solamente
            // in quel caso
            if (cmd != NULL) {
                printf("è un messaggio\n");
                if (strncmp(cmd, "#dest", 5) == 0) {
                    msgTo = strtok(cmd, " ");
                    msgTo = strtok(NULL, " ");
                    printf("privato verso %s: %s\n", msgTo, msg);
                } else if (strncmp(cmd, "#dest", 5) == 0) {
                    printf("broadcast\n");
                }
            }
*/

            printf("%s\n", msgToSend);

            if(send(sock , msgToSend , numChars , 0) < 0) {
                fprintf(stderr,"Cannot send the message to the server\n");
            }
        }


    } while(strncmp(msgToSend, "#logout", 7) != 0);

    close(sock);
    pthread_exit(NULL);
}

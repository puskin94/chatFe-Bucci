#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>


#include "include/chat-server.h"
#include "include/utils.h"
#include "include/hash.h"


// funzione suggerita in classe per la creazione di un timestamp

void timestamp(char * ts) {
    time_t t;
    t = time(NULL);
    ctime_r(&t,ts);
    ts[strlen(ts)-1] = '\0';
}

// consente la scrittura di messaggi sull'apposito file
void writeToLog(char *message) {

    FILE *fp;
    fp = fopen(logFile, "a");

    fprintf(fp,"%s", message);
    fclose(fp);

}

/* funzione che ha il compito di 'costruire' il testo per il file di log
includendo timestamp ed il messaggio stesso. Il messaggio viene stampato
anche su STDERR al variare dell' intero 'action' */
void buildLog(char *message, int action) {

    // if action == 0 -> normal message
    // if action == 1 -> error message

    char ts[64];
    char *errorMsg;

    timestamp(ts);
    errorMsg = malloc((strlen(ts) + strlen(message) + 2) * sizeof(char));
    sprintf(errorMsg, "%s:%s\n", ts, message);

    if (action == 1) {
        fprintf(stderr,errorMsg);
    }

    writeToLog(errorMsg);

}

/* funzione chiamata ad ogni avvio del server. Ha il compito di
verificare se il file di log esiste. In tal caso stampa su di esso
una intestazione che ne indica la data di avvio */
bool createLogFile() {

    FILE *fp;

    char ts[64];
    char *delimiter = "**************************************************\n";
    char *semiDelimiterL = "***  ";
    char *semiDelimiterR = " ***";
    char *firstMessage;
    timestamp(ts);

    firstMessage = malloc((strlen(delimiter) * 2 +
                            strlen(semiDelimiterL) +
                            strlen(semiDelimiterR) +
                            strlen(ts) +
                            strlen("Server Started @ ")) + 1 * sizeof(char));

    sprintf(firstMessage, "%s%sServer Started @ %s%s\n%s",
                                                delimiter,
                                                semiDelimiterL,
                                                ts,
                                                semiDelimiterR,
                                                delimiter);

    fp = fopen(logFile, "w");
    if (fp != NULL) {
        if (fprintf(fp,"%s\n", firstMessage)) {
            fclose(fp);
            return true;
        } else {
            fprintf(stderr,"Cannot write to log-file. Quitting...\n");
            return false;
        }
    } else {
        fprintf(stderr,"Cannot create log-file. Quitting...\n");
        return false;
    }
}

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
void writeToLog(char message[200]) {

    FILE *fp;
    fp = fopen(logFile, "a");

    fprintf(fp,"%s", message);
    fclose(fp);

}

/* funzione che ha il compito di 'costruire' il testo per il file di log
includendo timestamp ed il messaggio stesso. Il messaggio viene stampato
anche su STDERR al variare dell' intero 'action' */
void buildLog(char message[100], int action) {

    // if action == 0 -> normal message
    // if action == 1 -> error message

    char ts[64];
    char errorMsg[200] = "";
    timestamp(ts);
    strcat(errorMsg, ts);
    strcat(errorMsg, ":");
    strcat(errorMsg, message);
    strcat(errorMsg, "\n");

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
    char delimiter[52] = "**************************************************\n";
    char semiDelimiterL[5] = "***  ";
     char semiDelimiterR[4] = " ***";
    char firstMessage[300] = "";
    timestamp(ts);


    strcat(firstMessage, delimiter);
    strcat(firstMessage, semiDelimiterL);
    strcat(firstMessage, "Server Started @ ");
    strcat(firstMessage, ts);
    strcat(firstMessage, semiDelimiterR);
    strcat(firstMessage, "\n");
    strcat(firstMessage, delimiter);

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

void saveTable() {

    FILE *fp;
    hdata_t *user;

    fp = fopen(userFile, "w");
    if (fp != NULL) {
        user = (hdata_t *) malloc(sizeof(hdata_t));
        fprintf(fp, "%s:%s:%s\n", user->uname, user->fullname, user->email);
    }
    fclose(fp);

}

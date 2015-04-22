#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>

#include "chat-server.h"


void timestamp(char * ts) {
    time_t t;
    t = time(NULL);
    ctime_r(&t,ts);
    ts[strlen(ts)-1] = '\0';
}

void writeToLog(char message[200]) {

    FILE *fp;
    fp = fopen(logFile, "a");

    fprintf(fp,"%s\n", message);
    fclose(fp);

}

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


bool createLogFile() {

    FILE *fp;

    char ts[64];
    char delimiter[100] = "**************************************************\n";
    char semiDelimiterL[100] = "***  ";
     char semiDelimiterR[100] = " ***";
    char firstMessage[600] = "";
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

bool readUserFile() {

    FILE *fp;
    char *line = NULL;
    char userInfo[771]; // 768 == (256 * 3) + 2 + 1 --> (single max lenght + 2 (:) + 1 (\0))

    char *userName;
    char *fullName;
    char *mail;

    fp = fopen(userFile, "r+");
    if (fp != NULL) {
        while (fgets (userInfo, sizeof(userInfo), fp)) {
            userName = strtok (userInfo, ":");
            fullName = strtok (NULL, ":");
            mail = strtok (NULL, ":\n");

            if (userName != NULL && fullName != NULL && mail != NULL) {

                // Qua ci va l'inserimento dei dati nella struttuta ( hash table )

            }

        }

    } else {
        buildLog("Cannot load userFile. Quitting...", 1);
        return false;
    }

    fclose(fp);
    return true;

}

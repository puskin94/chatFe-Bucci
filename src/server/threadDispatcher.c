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


#include "include/chat-server.h"
#include "include/utils.h"
#include "include/hash.h"
#include "include/userManagement.h"

#define MSG_LOGIN 'L'
#define MSG_REGLOG 'R'
#define MSG_OK 'O'
#define MSG_ERROR 'E'
#define MSG_SINGLE 'S'
#define MSG_BRDCAST 'B'
#define MSG_LIST 'I'
#define MSG_LOGOUT 'X'


typedef struct {
    char type;
    char *sender;
    char *receiver;
    unsigned int msglen;
    char *msg;
} msg_t;


void *launchThreadDispatcher(void *newConn) {

    int sock = *(int*)newConn;
    printf("[Dispatcher] Sono connesso al socket n°%d\n", sock);

    pthread_exit(NULL);
}
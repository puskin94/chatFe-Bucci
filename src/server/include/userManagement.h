#ifndef USERMANAGEMENT_H
#define USERMANAGEMENT_H

#include <stdbool.h>

#include "common.h"
#include "lista.h"

bool readUserFile();
bool registerNewUser(char *msg);
bool isInTable(char *user);
int loginUser(char *user, int sock);
int returnSockId(char *user, hdata_t *bs);
void listUser(char **tmpBuff);
void logout(char *user);
void saveTable();

#endif

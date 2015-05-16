#ifndef USERMANAGEMENT_H
#define USERMANAGEMENT_H

#include <stdbool.h>

#include "common.h"
#include "lista.h"

bool readUserFile();
bool registerNewUser(char *msg, hdata_t *user);
int loginUser(char *user, hdata_t *bs, int sock);
void listUser();

#endif

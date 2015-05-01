/*************************************/
/* Library hash: schifano fabio 2013 */
/*************************************/

#ifndef HASH_H
#define HASH_H

#include <string.h>

#include "common.h"
#include "lista.h"



typedef lista * hash_t;


int hashfunc(char * k);


hdata_t * CERCALISTA ( char * key, lista L );


hash_t CREAHASH ();


void * CERCAHASH(char * key, hash_t H);


void INSERISCIHASH (char * key, hdata_t * elem, hash_t H);


#endif

#ifndef SERVER_DATABASE_H
#define SERVER_DATABASE_H

#include "rpc.h"
#include "defines.h"

typedef struct _SKEL_RECORD_ {
    char *fct_name;
    int *arg_types;
    unsigned int arg_types_len;
    skeleton skel;
    _SKEL_RECORD_* next;
} SKEL_RECORD;

extern SKEL_RECORD* skel_db_head;

// insert a new skeleton record into db
int skel_db_put(SKEL_RECORD sig);

// remove the specified skeleton record in db
int skel_db_delete(SKEL_RECORD sig);

// find skeleton using given fct_name and arg_types, store result in skel
int skel_db_get(skeleton& skel, char* fct_name, int *arg_types);

// print out the contents of the database
int skel_db_print();

#endif // SERVER_DATBASE_H

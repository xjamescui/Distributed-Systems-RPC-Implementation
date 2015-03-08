#ifndef DATABASE_H
#define DATABASE_H

#include "defines.h"

// save a list of server_fd that are active
typedef struct {
    char *fct_name;         // fct name
    unsigned int num_args;  // num of args
    int *arg_types;         // arg types
} SIGNATURE;
typedef struct _HOST_ {
    unsigned int ip;        // host's ip
    unsigned int port;      // host's port
    _HOST_* next;             // pointer to next HOST
} HOST;
typedef struct _DB_NODE_ {
    SIGNATURE sig;          // one signature
    HOST* hosts_root;       // a queue of hosts
    _DB_NODE_* next;          // pointer to next DB_NODE
} DB_NODE;

extern DB_NODE* g_db_nodes_root;

int db_put(unsigned int ip, unsigned int port, SIGNATURE sig);
int db_get(unsigned int *ip, unsigned int *port, SIGNATURE sig);
int db_delete_host(unsigned int ip, unsigned int port, SIGNATURE sig);
int db_drop();



#endif // DATABASE_H

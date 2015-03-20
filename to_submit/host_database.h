#ifndef HOST_DATABASE_H
#define HOST_DATABASE_H

#include "defines.h"

// save a list of server_fd that are active
typedef struct {
    unsigned int fct_name_len; // length of fct name
    char *fct_name;         // fct name
    unsigned int arg_types_len;  // num of args
    int *arg_types;         // arg types
} SIGNATURE;

typedef struct _HOST_ {
    int sock_fd;            // host's socket_fd
    unsigned int ip;        // host's ip
    unsigned int port;      // host's port
    _HOST_* next;           // pointer to next HOST
} HOST;

typedef struct _DB_NODE_ {
    SIGNATURE sig;          // one signature
    HOST* hosts_root;       // a queue of hosts
    _DB_NODE_* next;        // pointer to next DB_NODE
} DB_NODE;

extern DB_NODE* g_db_nodes_root;

int db_put(const HOST &host, SIGNATURE sig);
int db_get(HOST* host, SIGNATURE sig);
int db_delete_host(const HOST &host, SIGNATURE sig);
int db_delete_all_for_sock(const int sock_fd);

int db_drop();

int db_get_all(unsigned int *hosts_len, unsigned int **ips, unsigned int **ports, SIGNATURE sig);

int db_print();
unsigned int db_size();

#endif // HOST_DATABASE_H

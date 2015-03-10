#include "rpc.h"
#include "helper.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "debug.h"

#define MAX_CLIENT_CONNECTIONS 100

int client_listener_fd, binder_fd; // SERVER: client listener socket, binder connection socket

/** create sockets and connect to the binder
 *
 * return:
 * 0 - success
 * -1 - error
 */
int rpcInit() {

    struct sockaddr_in client_addr, binder_addr;
    unsigned int client_addr_len = sizeof(client_addr);
    unsigned int binder_addr_len = sizeof(binder_addr);
    struct hostent* binder_hostinfo;
    int binder_port;
    char* binder_address;

    // check binder environment information are set
    if (getenv("BINDER_ADDRESS") == NULL || getenv("BINDER_PORT") == NULL) {
        fprintf(stderr, "Please ensure BINDER_ADDRESS and BINDER_PORT environment variables are set\n");
        return -1;
    }

    binder_port = atoi(getenv("BINDER_PORT"));
    binder_address = getenv("BINDER_ADDRESS");

    // create client listener socket
    client_listener_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_listener_fd < 0) {
        fprintf(stderr, "ERROR creating client socket on server: %s\n", strerror(errno));
        return -1;
    }

    // prep client listener socket info for binding
    memset(&client_addr, '0', client_addr_len);
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = INADDR_ANY;

    // bind client listener socket
    if ( bind(client_listener_fd, (const struct sockaddr *)(&client_addr), client_addr_len) < 0 ) {
        fprintf(stderr,"ERROR binding client listener socket: %s\n", strerror(errno));
        return -1;
    }

    // create binder connection socket
    binder_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (binder_fd < 0) {
        fprintf(stderr, "ERROR creating binder connection socket on server: %s\n", strerror(errno));
        return -1;
    }

    // prep binder connection socket info to connect to binder
    binder_hostinfo = gethostbyname(binder_address);
    memset(&binder_addr, '0', binder_addr_len);
    memcpy(&binder_addr.sin_addr.s_addr, binder_hostinfo->h_addr , binder_hostinfo->h_length);
    binder_addr.sin_family = AF_INET;
    binder_addr.sin_port = htons(binder_port);

    // connect to binder
    if ( connect(binder_fd, (const struct sockaddr*)(&binder_addr), binder_addr_len) < 0 ) {
        fprintf(stderr, "ERROR connecting to binder from server: %s\n", strerror(errno));
        return -1;
    }

    DEBUG("CONNECTED to %s\n", binder_hostinfo->h_name);

    return 0;
}

int rpcCall(char* name, int* argTypes, void** args) {
    return -1;
}

int rpcCacheCall(char* name, int* argTypes, void** args) {
    return -1;
}

int rpcRegister(char* name, int* argTypes, skeleton f) {
    return -1;
}

int rpcExecute() {
    return -1;
}

int rpcTerminate() {
    return -1;
}

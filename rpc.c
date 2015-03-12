#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "debug.h"
#include "defines.h"
#include "helper.h"
#include "rpc.h"

#define MAX_CLIENT_CONNECTIONS 100

int server_fd, binder_fd; // SERVER: client listener socket, binder connection socket

/**
 * create sockets and connect to the binder
 *
 * return:
 * 0  = success
 * <0 = error
 */
int rpcInit() {

    struct sockaddr_in server_addr, binder_addr;
    unsigned int server_addr_len = sizeof(server_addr);
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
    server_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd < 0) {
        fprintf(stderr, "ERROR creating client socket on server: %s\n", strerror(errno));
        return -1;
    }

    // prep client listener socket info for binding
    memset(&server_addr, '0', server_addr_len);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // bind client listener socket
    if ( bind(server_fd, (const struct sockaddr *)(&server_addr), server_addr_len) < 0 ) {
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

/** 
 * register a function on binder and add skeleton record into local database
 *
 * returns:
 * 0  = successful registration
 * >0 = warning
 * <0 = failure
 */
int rpcRegister(char* name, int* argTypes, skeleton f) {

    unsigned int server_ip, server_port, num_args;
    char* msg = NULL;
    int msg_len;

    server_ip = 0;
    server_port = 0;

    // insert skeleton and name and argTypes into local database

    // create MSG_REGISTER type msg
    // format: msg_len, msg_type, server_ip, server_port, fct_name, num_args, argTypes
    if (assemble_msg(&msg, &msg_len, MSG_REGISTER, server_ip, server_port, name, num_args, argTypes) < 0){
        fprintf(stderr, "ERROR creating registration request message\n");
        return -1;
    }



    return 0;
}

int rpcExecute() {

    return -1;
}

int rpcTerminate() {
    return -1;
}

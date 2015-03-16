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
#include "SkeletonDatabase.h"


using namespace std;

int rpcCall(char* name, int* argTypes, void** args)
{
    // declare sock vars
    int binder_fd;
    char* binder_address;
    char* binder_port_str;
    int binder_port;

    // declare vars for buffer
    char* w_buffer = NULL;
    unsigned int w_buffer_len;
    ssize_t write_len;

    // get environment variables
    binder_address = getenv(BINDER_ADDRESS_STRING);
    binder_port_str = getenv(BINDER_PORT_STRING);
    if (binder_address == NULL || binder_port_str == NULL) {
        fprintf(stderr, "Error : rpcTerminate() BINDER_ADDRESS and/or BINDER_PORT not set\n");
        return -1;
    }
    binder_port = atoi(binder_port_str);

    // connect to binder
    if ( connect_to_hostname_port(&binder_fd, binder_address, binder_port) < 0 ) {
        fprintf(stderr, "Error : rpcTerminate() cannot connect to binder\n");
        return -1;
    }

    //

    return -1;
}

int rpcCacheCall(char* name, int* argTypes, void** args)
{
    return -1;
}


int rpcTerminate()
{
    // declare sock vars
    int binder_fd;
    char* binder_address;
    char* binder_port_str;
    int binder_port;

    // declare vars for buffer
    char* w_buffer = NULL;
    unsigned int w_buffer_len;
    ssize_t write_len;

    // get environment variables
    binder_address = getenv(BINDER_ADDRESS_STRING);
    binder_port_str = getenv(BINDER_PORT_STRING);
    if (binder_address == NULL || binder_port_str == NULL) {
        fprintf(stderr, "Error : rpcTerminate() BINDER_ADDRESS and/or BINDER_PORT not set\n");
        return -1;
    }
    binder_port = atoi(binder_port_str);

    // connect to binder
    if ( connect_to_hostname_port(&binder_fd, binder_address, binder_port) < 0 ) {
        fprintf(stderr, "Error : rpcTerminate() cannot connect to binder\n");
        return -1;
    }

    // assemble a terminate message
    if ( assemble_msg(&w_buffer,&w_buffer_len,MSG_TERMINATE) < 0 ) {
        fprintf(stderr,"Error : rpcTerminate() couldn't assemble terminate message\n");
        return -1;
    }

    // send message
    write_len = write_message(g_binder_fd, w_buffer, w_buffer_len);
    free(w_buffer);
    if ( write_len < w_buffer_len ) {
        fprintf(stderr,"Error : rpcTerminate() couldn't send terminate to binder\n");
        return -1;
    }

    close(binder_fd);
    return 0;
}




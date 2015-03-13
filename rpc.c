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
#include "skeleton_database.h"

#define MAX_CLIENT_CONNECTIONS 100

unsigned int g_binder_fd;
unsigned int g_server_fd, g_server_port, g_server_ip;

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
    g_server_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_server_fd < 0) {
        fprintf(stderr, "ERROR creating client socket on server: %s\n", strerror(errno));
        return -1;
    }

    // prep client listener socket info for binding
    memset(&server_addr, '0', server_addr_len);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // bind client listener socket
    if ( bind(g_server_fd, (const struct sockaddr *)(&server_addr), server_addr_len) < 0 ) {
        fprintf(stderr,"ERROR binding client listener socket: %s\n", strerror(errno));
        return -1;
    }

    // set server ip and port to global variable
    g_server_port = server_addr.sin_port;
    if(get_ip_from_socket(&g_server_ip, g_server_fd) < 0){
        return -1;
    }

    // create binder connection socket
    g_binder_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (g_binder_fd < 0) {
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
    if ( connect(g_binder_fd, (const struct sockaddr*)(&binder_addr), binder_addr_len) < 0 ) {
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

    unsigned int num_args, msg_len, name_len, write_len;
    char* msg = NULL;
    name_len = strlen(name);
    num_args = 4; // TODO change this later: need to know how to determine length of int*

    DEBUG("fct_name is %s\n", name);
    DEBUG("fct_name_len is %d\n", name_len);
    // insert skeleton and name and argTypes into local database
    /* skel_record.fct_name = name; */
    /* skel_record.arg_types = argTypes; */
    /* skel_record.skel = f; */

    /* db_op_result = skel_db_put(skel_record); */

    /* if (db_op_result < 0) { */
    /*   // ERROR inserting record */
    /*   return db_op_result; */
    /* } */

    // create MSG_REGISTER type msg
    // format: msg_len, msg_type, server_ip, server_port, fct_name_len, fct_name, num_args, argTypes
    if (assemble_msg(&msg, &msg_len, MSG_REGISTER, g_server_ip, g_server_port, name_len, name, num_args, argTypes) < 0){
        fprintf(stderr, "ERROR creating registration request message\n");
        return -1;
    }

    // send registration message to binder
    write_len = write_large(g_binder_fd,msg,msg_len);
    if ( write_len < msg_len ) {
        fprintf(stderr, "Error : couldn't send register request\n");
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

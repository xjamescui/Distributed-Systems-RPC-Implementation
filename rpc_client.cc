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

/**
 * rpcCall helper functions
 *
 *
 */
int ask_binder_for_host(int binder_fd, unsigned int *ip, unsigned int *port,
                        unsigned int name_len, char *name,
                        unsigned int arg_types_len, int *arg_types);
int send_execute_to_server(int server_fd,
                           unsigned int name_len, char *name,
                           unsigned int arg_types_len, int *arg_types, void** args);

/**
 * error codes:
 * RPC_ENVR_VARIABLES_NOT_SET
 * RPC_CONNECT_TO_HOST_FAIL
 * RPC_CONNECT_TO_IP_FAIL
 */
int rpcCall(char* name, int* argTypes, void** args)
{

    DEBUG("rpcCall %s",name);

    int opCode;

    // declare sock vars
    int binder_fd;
    char* binder_address;
    char* binder_port_str;
    unsigned int binder_port;
    unsigned short binder_port_short;

    // server stuff
    int server_fd;
    unsigned int server_ip;
    unsigned int server_port;
    unsigned short server_port_short;


    // get the length of name and argTypes, since it will be used couple of times
    unsigned int name_len = strlen(name);
    unsigned int argTypesLen = arg_types_length(argTypes);

    // get environment variables
    binder_address = getenv(BINDER_ADDRESS_STRING);
    binder_port_str = getenv(BINDER_PORT_STRING);
    if (binder_address == NULL || binder_port_str == NULL) {
        fprintf(stderr, "Error : rpcCall() BINDER_ADDRESS and/or BINDER_PORT not set\n");
        return RPC_ENVR_VARIABLES_NOT_SET;
    }
    binder_port = atoi(binder_port_str);
    binder_port_short = binder_port;
    binder_port_short = htons(binder_port_short);

    // connect to binder
    if ( connect_to_hostname_port(&binder_fd, binder_address, binder_port_short) < 0 ) {
        fprintf(stderr, "Error : rpcCall() cannot connect to binder\n");
        return RPC_CONNECT_TO_HOST_FAIL;
    }

    // get ip and port from binder
    if ( (opCode = ask_binder_for_host(binder_fd,&server_ip,&server_port,name_len,name,argTypesLen,argTypes)) < 0 ) {
        // fprintf(stderr, "Error : rpcCall() cannot get host\n");
        DEBUG("Error : rpcCall() cannot get host %d\n",opCode);
        return opCode;
    }

    // done with binder, close it
    close(binder_fd);


    // TODO: remove this {
    unsigned int ntohip = ntohl(server_ip);
    unsigned char ipb1,ipb2,ipb3,ipb4;
    ipb1 = (ntohip >> 24) & 0xFF;
    ipb2 = (ntohip >> 16) & 0xFF;
    ipb3 = (ntohip >> 8) & 0xFF;
    ipb4 = (ntohip >> 0) & 0xFF;
    DEBUG("calling server: %u.%u.%u.%u:%u",ipb1,ipb2,ipb3,ipb4,ntohs(server_port));
    // }

    // connect to server
    server_port_short = server_port;
    if ( connect_to_ip_port(&server_fd, server_ip, server_port_short) < 0 ) {
        fprintf(stderr, "Error : rpcCall() cannot connect to server %x:%u\n",server_ip,server_port_short);
        return RPC_CONNECT_TO_IP_FAIL;
    }

    DEBUG("connect success!");

    // execute the message
    if ( (opCode = send_execute_to_server(server_fd, name_len, name, argTypesLen, argTypes, args)) < 0 ) {
        fprintf(stderr, "Error : rpcCall() cannot execute %d\n",opCode);
        return opCode;
    }

    close(server_fd);
    return 0;
}

int rpcCacheCall(char* name, int* argTypes, void** args)
{
    return -1;
}

/**
 * error codes:
 * RPC_ENVR_VARIABLES_NOT_SET
 * RPC_CONNECT_TO_HOST_FAIL
 * ASSEMBLE_MSG_FAIL
 * RPC_WRITE_MSG_FAIL
 *
 */
int rpcTerminate()
{
    // declare sock vars
    int binder_fd;
    char* binder_address;
    char* binder_port_str;
    unsigned int binder_port;
    unsigned short binder_port_short;

    // declare vars for buffer
    char* w_buffer = NULL;
    unsigned int w_buffer_len;
    ssize_t write_len;

    // get environment variables
    binder_address = getenv(BINDER_ADDRESS_STRING);
    binder_port_str = getenv(BINDER_PORT_STRING);
    if (binder_address == NULL || binder_port_str == NULL) {
        fprintf(stderr, "Error : rpcTerminate() BINDER_ADDRESS and/or BINDER_PORT not set\n");
        return RPC_ENVR_VARIABLES_NOT_SET;
    }
    binder_port = atoi(binder_port_str);
    binder_port_short = binder_port;
    binder_port_short = htons(binder_port_short);

    // connect to binder
    if ( connect_to_hostname_port(&binder_fd, binder_address, binder_port_short) < 0 ) {
        fprintf(stderr, "Error : rpcTerminate() cannot connect to binder\n");
        return RPC_CONNECT_TO_HOST_FAIL;
    }

    // assemble a terminate message
    assemble_msg(&w_buffer,&w_buffer_len,MSG_TERMINATE);

    // send message
    write_len = write_message(binder_fd, w_buffer, w_buffer_len);
    free(w_buffer);
    if ( write_len < w_buffer_len ) {
        fprintf(stderr,"Error : rpcTerminate() couldn't send terminate to binder\n");
        return RPC_WRITE_MSG_FAIL;
    }

    close(binder_fd);
    return 0;
}

/**
 * rpcCall helper functions
 *
 * WRITE_MSG_FAIL
 * READ_MSG_FAIL
 * MSG_TYPE_NOT_SUPPORTED
 *
 */
int ask_binder_for_host(int binder_fd, unsigned int *ip, unsigned int *port,
                        unsigned int name_len, char *name,
                        unsigned int arg_types_len, int *arg_types)
{

    // for the buffer
    char* rw_buffer = NULL;
    unsigned int rw_buffer_len;
    ssize_t read_len, write_len;

    // stuff to be extracted
    unsigned int msg_len;
    char msg_type;
    unsigned server_ip, server_port;
    int result;

    // assemble
    assemble_msg(&rw_buffer,&rw_buffer_len,MSG_LOC_REQUEST,
                 name_len,name,arg_types_len,arg_types);

    // send loc request to binder
    write_len = write_message(binder_fd,rw_buffer,rw_buffer_len);
    free(rw_buffer);
    rw_buffer = NULL;
    if ( write_len < rw_buffer_len ) {
        fprintf(stderr, "Error : couldn't send loc request\n");
        return WRITE_MSG_FAIL;
    }

    // wait for answer
    read_len = read_message(&rw_buffer,binder_fd);
    if ( read_len < 0 ) {
        fprintf(stderr, "Error : couldn't read reply of loc request\n");
        return READ_MSG_FAIL;
    }

    // extract length and type
    extract_msg_len_type(&msg_len,&msg_type,rw_buffer);
    rw_buffer_len = msg_len + 5;

    // check type
    switch (msg_type) {
    case MSG_LOC_SUCCESS : {
        extract_msg(rw_buffer,rw_buffer_len,MSG_LOC_SUCCESS,&server_ip,&server_port);
        free(rw_buffer);
        *ip = server_ip;
        *port = server_port;
    }
    break;
    case MSG_LOC_FAILURE : {
        extract_msg(rw_buffer,rw_buffer_len,MSG_LOC_FAILURE,&result);
        free(rw_buffer);
        return result; // TODO : check if this is really what we want to return
    }
    break;
    default:
        fprintf(stderr, "Error : asking for server ip and port, got invalid type %x\n",msg_type);
        free(rw_buffer);
        return MSG_TYPE_NOT_SUPPORTED;
    }

    // success!
    return 0;
}

/**
 *  Send execute request to server
 *
 *  error codes:
 *  WRITE_MSG_FAIL
 *  READ_MSG_FAIL
 *  MSG_TYPE_NOT_SUPPORTED
 */
int send_execute_to_server(int server_fd,
                           unsigned int name_len, char *name,
                           unsigned int arg_types_len, int *arg_types, void** args)
{

    // for the buffer
    char* rw_buffer = NULL;
    unsigned int rw_buffer_len;
    ssize_t read_len, write_len;

    // for the message
    unsigned int msg_len;
    char msg_type;
    unsigned int reply_name_len; // not sure if these are needed
    char* reply_name = NULL; // not sure if these are needed
    unsigned int reply_arg_types_len; // not sure if these are needed
    int* reply_arg_types = NULL; // not sure if these are needed
    void** reply_args = NULL;
    int result;

    // assemble message
    assemble_msg(&rw_buffer,&rw_buffer_len,MSG_EXECUTE,
                 name_len,name,arg_types_len,arg_types,args);

    // send to server
    write_len = write_message(server_fd,rw_buffer,rw_buffer_len);
    free(rw_buffer);
    rw_buffer = NULL;
    if ( write_len < rw_buffer_len) {
        fprintf(stderr, "Error : couldn't send execute\n");
        return WRITE_MSG_FAIL;
    }

    // wait for answer
    read_len = read_message(&rw_buffer,server_fd);
    if ( read_len < 0 ) {
        fprintf(stderr, "Error : couldn't read reply of loc request\n");
        return READ_MSG_FAIL;
    }

    // extract len and type
    extract_msg_len_type(&msg_len,&msg_type,rw_buffer);
    rw_buffer_len = msg_len + 5;

    switch ( msg_type ) {
    case MSG_EXECUTE_SUCCESS: {
        extract_msg(rw_buffer,rw_buffer_len,MSG_EXECUTE_SUCCESS,
                    &reply_name_len,&reply_name,&reply_arg_types_len,&reply_arg_types,&reply_args);
        free(rw_buffer);
    }
    break;
    case MSG_EXECUTE_FAILURE: {
        extract_msg(rw_buffer,rw_buffer_len,MSG_EXECUTE_FAILURE,&result);
        free(rw_buffer);
        return result;
    }
    break;
    default:
        free(rw_buffer);
        fprintf(stderr, "Error : asking for server execute, got invalid type %x\n",msg_type);
        return MSG_TYPE_NOT_SUPPORTED;
    }

    // copy args over
    copy_args_step_by_step(reply_arg_types, args, reply_args);

    free(reply_name);
    for ( unsigned int i = 0 ; i < reply_arg_types_len ; i += 1 ) {
        if ( type_is_array(reply_arg_types[i]) ) {
            free(reply_args[i]);
        }
    }
    free(reply_arg_types);
    free(reply_args);


    return 0;
}

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
#include "ClientCacheDatabase.h"

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
int ask_binder_for_cache_host(int binder_fd, unsigned int *hosts_len, 
                              unsigned int **ips, unsigned int **ports,
                              unsigned int name_len, char *name,
                              unsigned int arg_types_len, int *arg_types);
/**
 * success:
 * RPC_CALL_SUCCESS
 * error codes:
 * RPC_NULL_PARAMETERS
 * RPC_INVALID_ARGTYPES
 * RPC_ENVR_VARIABLES_NOT_SET
 * RPC_CONNECT_TO_BINDER_FAIL
 * RPC_CONNECT_TO_SERVER_FAIL
 * RPC_CALL_NO_HOSTS
 * or failure code from the actual function
 */
int rpcCall(char* name, int* argTypes, void** args)
{

    if ( name == NULL || argTypes == NULL || args == NULL ) {
        return RPC_NULL_PARAMETERS;
    }

    if ( validate_arg_types(argTypes) < 0 ) {
        return RPC_INVALID_ARGTYPES;
    }

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
        return RPC_CONNECT_TO_BINDER_FAIL;
    }

    // get ip and port from binder
    if ( (opCode = ask_binder_for_host(binder_fd,&server_ip,&server_port,name_len,name,argTypesLen,argTypes)) < 0 ) {
        fprintf(stderr, "Error : rpcCall() cannot get server from binder %d\n",opCode);
        return opCode;
    }

    // done with binder, close it
    close(binder_fd);

    // connect to server
    server_port_short = server_port;
    if ( connect_to_ip_port(&server_fd, server_ip, server_port_short) < 0 ) {
        fprintf(stderr, "Error : rpcCall() cannot connect to server %x:%u\n",server_ip,server_port_short);
        return RPC_CONNECT_TO_SERVER_FAIL;
    }

    DEBUG("connect to server success!");

    // execute the message
    if ( (opCode = send_execute_to_server(server_fd, name_len, name, argTypesLen, argTypes, args)) < 0 ) {
        fprintf(stderr, "Error : rpcCall() cannot execute %d\n",opCode);
        return opCode;
    }

    close(server_fd);
    return RPC_CALL_SUCCESS;
}

/**
 * success:
 * RPC_CACHE_CALL_SUCCESS
 * error codes:
 * RPC_NULL_PARAMETERS
 * RPC_INVALID_ARGTYPES
 * RPC_ENVR_VARIABLES_NOT_SET
 * RPC_CONNECT_TO_BINDER_FAIL
 * RPC_CONNECT_TO_SERVER_FAIL
 * RPC_CALL_NO_HOSTS
 * or failure code from the actual function
 */
int rpcCacheCall(char* name, int* argTypes, void** args)
{

    if ( name == NULL || argTypes == NULL || args == NULL ) {
        return RPC_NULL_PARAMETERS;
    }

    if ( validate_arg_types(argTypes) < 0 ) {
        return RPC_INVALID_ARGTYPES;
    }

    // setup
    SIGNATURE sig;
    sig.fct_name_len = strlen(name);
    sig.fct_name = name;
    sig.arg_types_len = arg_types_length(argTypes);
    sig.arg_types = argTypes;

    HOST host = { 0 , 0 , 0 , 0 };

    // check if it is in cache
    int get_code;
    get_code = ClientCacheDatabase::Instance()->get(&host,sig);

    // if it's not, then ask binder again
    int opCode;
    if ( get_code < 0 ) {
        DEBUG("asking binder for cache");

        // declare binder sock vars
        int binder_fd;
        char* binder_address;
        char* binder_port_str;
        unsigned int binder_port;
        unsigned short binder_port_short;

        // server stuff that get returned by binder
        unsigned int hosts_len;
        unsigned int* server_ips = NULL;
        unsigned int* server_ports = NULL;

        // get environment variables
        binder_address = getenv(BINDER_ADDRESS_STRING);
        binder_port_str = getenv(BINDER_PORT_STRING);
        if (binder_address == NULL || binder_port_str == NULL) {
            fprintf(stderr, "Error : rpcCacheCall() BINDER_ADDRESS and/or BINDER_PORT not set\n");
            return RPC_ENVR_VARIABLES_NOT_SET;
        }
        binder_port = atoi(binder_port_str);
        binder_port_short = binder_port;
        binder_port_short = htons(binder_port_short);

        // connect to binder
        if ( connect_to_hostname_port(&binder_fd, binder_address, binder_port_short) < 0 ) {
            fprintf(stderr, "Error : rpcCacheCall() cannot connect to binder\n");
            return RPC_CONNECT_TO_BINDER_FAIL;
        }

        // get ip and port from binder
        if ( (opCode = ask_binder_for_cache_host(binder_fd,
                            &hosts_len, &server_ips,&server_ports,
                            sig.fct_name_len,sig.fct_name,
                            sig.arg_types_len,sig.arg_types)) < 0 ) {
            fprintf(stderr, "Error : rpcCacheCall() cannot get cache servers from binder %d\n",opCode);
            return opCode;
        }

        // done with binder, close it
        close(binder_fd);

        // put everything in cache database, pretty sure hosts_len > 0
        for ( unsigned int i = 0 ; i < hosts_len ; i += 1 ) {
            host.ip = server_ips[i];
            host.port = server_ports[i];
            ClientCacheDatabase::Instance()->put(host,sig);
        }

        DEBUG("got new cache, db size:%d",ClientCacheDatabase::Instance()->size());

        // call get again, there is at least one
        get_code = ClientCacheDatabase::Instance()->get(&host,sig);
        if ( get_code < 0 ) {
            fprintf(stderr, "Error : rpcCacheCall() should not reach here %d\n",get_code);
            return RPC_CALL_INTERNAL_DB_ERROR;
        }

    }

    // at this point, we got at least 1 server
    // declare server stuff
    int server_fd;
    unsigned int server_ip = host.ip;
    unsigned short server_port_short = host.port;

    // connect to server
    if ( connect_to_ip_port(&server_fd, server_ip, server_port_short) < 0 ) {
        fprintf(stderr, "Error : rpcCacheCall() cannot connect to server %x:%u\n",server_ip,server_port_short);
        return RPC_CONNECT_TO_SERVER_FAIL;
    }

    DEBUG("connect to server success!");

    // execute the message
    if ( (opCode = send_execute_to_server(server_fd, sig.fct_name_len, sig.fct_name, sig.arg_types_len, sig.arg_types, args)) < 0 ) {
        fprintf(stderr, "Error : rpcCacheCall() cannot execute %d\n",opCode);
        return opCode;
    }

    close(server_fd);

    return RPC_CACHE_CALL_SUCCESS;
}

/**
 * error codes:
 * RPC_ENVR_VARIABLES_NOT_SET
 * RPC_CONNECT_TO_BINDER_FAIL
 * RPC_WRITE_TO_BINDER_FAIL
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
        return RPC_CONNECT_TO_BINDER_FAIL;
    }

    // assemble a terminate message
    assemble_msg(&w_buffer,&w_buffer_len,MSG_TERMINATE);

    // send message
    write_len = write_message(binder_fd, w_buffer, w_buffer_len);
    free(w_buffer);
    if ( write_len < w_buffer_len ) {
        fprintf(stderr,"Error : rpcTerminate() couldn't send terminate to binder\n");
        return RPC_WRITE_TO_BINDER_FAIL;
    }

    close(binder_fd);
    return RPC_TERMINATE_SUCCESS;
}


/**
 * rpcCall helper functions
 *
 * RPC_WRITE_TO_BINDER_FAIL
 * RPC_READ_FROM_BINDER_FAIL
 * RPC_CALL_NO_HOSTS
 * MSG_TYPE_NOT_SUPPORTED
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

    // assemble
    assemble_msg(&rw_buffer,&rw_buffer_len,MSG_LOC_REQUEST,
                 name_len,name,arg_types_len,arg_types);

    // send loc request to binder
    write_len = write_message(binder_fd,rw_buffer,rw_buffer_len);
    free(rw_buffer);
    rw_buffer = NULL;
    if ( write_len < rw_buffer_len ) {
        fprintf(stderr, "Error : couldn't send loc request\n");
        return RPC_WRITE_TO_BINDER_FAIL;
    }

    // wait for answer
    read_len = read_message(&rw_buffer,binder_fd);
    if ( read_len < 0 ) {
        fprintf(stderr, "Error : couldn't read reply of loc request\n");
        return RPC_READ_FROM_BINDER_FAIL;
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
        int result;
        extract_msg(rw_buffer,rw_buffer_len,MSG_LOC_FAILURE,&result);
        free(rw_buffer);
        switch(result){
        case MSG_LOC_FAILURE_SIGNATURE_NOT_FOUND :
        case MSG_LOC_FAILURE_SIGNATURE_NO_HOSTS :
            return RPC_CALL_NO_HOSTS;
        }
    }
    break;
    default:
        fprintf(stderr, "Error : asking for server ip and port, got invalid type %x\n",msg_type);
        free(rw_buffer);
        return MSG_TYPE_NOT_SUPPORTED;
    }

    // success!
    return RPC_CALL_SUCCESS;
}

/**
 *  Send execute request to server
 *
 *  error codes:
 *  RPC_WRITE_TO_SERVER_FAIL
 *  RPC_READ_FROM_SERVER_FAIL
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
    unsigned int reply_name_len;
    char* reply_name = NULL;
    unsigned int reply_arg_types_len;
    int* reply_arg_types = NULL;
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
        return RPC_WRITE_TO_SERVER_FAIL;
    }

    // wait for answer
    read_len = read_message(&rw_buffer,server_fd);
    if ( read_len < 0 ) {
        fprintf(stderr, "Error : couldn't read reply of loc request\n");
        return RPC_READ_FROM_SERVER_FAIL;
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
        free(reply_args[i]);
    }
    free(reply_arg_types);
    free(reply_args);


    return 0;
}


/**
 *  Send execute request to server
 *
 *  error codes:
 *  RPC_WRITE_TO_SERVER_FAIL
 *  RPC_READ_FROM_SERVER_FAIL
 *  MSG_TYPE_NOT_SUPPORTED
 */
int ask_binder_for_cache_host(int binder_fd, unsigned int *hosts_len, 
                              unsigned int **ips, unsigned int **ports,
                              unsigned int name_len, char *name,
                              unsigned int arg_types_len, int *arg_types) {

    // for the buffer
    char* rw_buffer = NULL;
    unsigned int rw_buffer_len;
    ssize_t read_len, write_len;

    // stuff to be extracted
    unsigned int msg_len;
    char msg_type;
    unsigned int *server_ips = NULL;
    unsigned int *server_ports = NULL;

    // assemble
    assemble_msg(&rw_buffer,&rw_buffer_len,MSG_LOC_CACHE_REQUEST,
                 name_len,name,arg_types_len,arg_types);

    // send loc request to binder
    write_len = write_message(binder_fd,rw_buffer,rw_buffer_len);
    free(rw_buffer);
    rw_buffer = NULL;
    if ( write_len < rw_buffer_len ) {
        fprintf(stderr, "Error : couldn't send loc cache request\n");
        return RPC_WRITE_TO_BINDER_FAIL;
    }

    // wait for answer
    read_len = read_message(&rw_buffer,binder_fd);
    if ( read_len ==  READ_MSG_FAIL ) {
        fprintf(stderr, "Error : couldn't read reply of loc cache request\n");
        return RPC_READ_FROM_BINDER_FAIL;
    }
    if ( read_len ==  READ_MSG_ZERO_LENGTH ) {
        fprintf(stderr, "Error : binder socket closed\n");
        return RPC_READ_FROM_BINDER_FAIL;
    }

    // extract length and type
    extract_msg_len_type(&msg_len,&msg_type,rw_buffer);
    rw_buffer_len = msg_len + 5;

    // check type
    switch (msg_type) {
    case MSG_LOC_CACHE_SUCCESS : {
        extract_msg(rw_buffer,rw_buffer_len,MSG_LOC_CACHE_SUCCESS,
                    hosts_len,&server_ips,&server_ports);
        free(rw_buffer);
        *ips = (unsigned int*)malloc((*hosts_len)*sizeof(unsigned int));
        *ports = (unsigned int*)malloc((*hosts_len)*sizeof(unsigned int));
        for ( unsigned int i = 0 ; i < (*hosts_len) ; i += 1 ) {
            (*ips)[i] = server_ips[i];
            (*ports)[i] = server_ports[i];
        }
        free(server_ips);
        free(server_ports);
    }
    break;
    case MSG_LOC_CACHE_FAILURE : {
        int result;
        extract_msg(rw_buffer,rw_buffer_len,MSG_LOC_CACHE_FAILURE,&result);
        free(rw_buffer);
        switch(result){
        case MSG_LOC_FAILURE_SIGNATURE_NOT_FOUND :
        case MSG_LOC_FAILURE_SIGNATURE_NO_HOSTS :
            return RPC_CALL_NO_HOSTS;
        }
    }
    break;
    default:
        fprintf(stderr, "Error : asking for server ip and port, got invalid type 0x%x\n",msg_type);
        free(rw_buffer);
        return MSG_TYPE_NOT_SUPPORTED;
    }

    // success!
    return RPC_CACHE_CALL_SUCCESS;
}


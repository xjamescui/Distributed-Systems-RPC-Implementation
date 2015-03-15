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

#define MAX_CLIENT_CONNECTIONS 100


unsigned int g_binder_fd;
unsigned int g_server_fd, g_server_port, g_server_ip;
SkeletonDatabase* g_skeleton_database;


int create_server_socket();
int connect_to_binder();
void handle_binder_message(char* msg, unsigned int binder_fd);
void handle_client_message(char* msg, unsigned int client_fd);

/**
 * create sockets and connect to the binder
 *
 * return:
 * 0  = success
 * <0 = error
 */
int rpcInit()
{

    int binderOpCode, serverOpCode;

    // create binder socket and connect to binder
    if((binderOpCode = connect_to_binder()) < 0) return binderOpCode;

    // create server socket to listen to client
    if((serverOpCode = create_server_socket()) < 0) return serverOpCode;

    // initialize skeleton database
    g_skeleton_database = new SkeletonDatabase();

    return 0;
} // rpcInit


int rpcCall(char* name, int* argTypes, void** args)
{
    return -1;
}

int rpcCacheCall(char* name, int* argTypes, void** args)
{
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
int rpcRegister(char* name, int* argTypes, skeleton f)
{

    unsigned int num_args, msg_len, name_len, write_len;
    char* msg = NULL;
    SKEL_RECORD skel_record;
    int dbOpCode;

    name_len = strlen(name);
    num_args = arg_types_length(argTypes);

    // insert skeleton and name and argTypes into local database
    skel_record.fct_name = name;
    skel_record.arg_types = argTypes;
    skel_record.skel = f;

    DEBUG("NOW INSERTING: %s\n", skel_record.fct_name);
    dbOpCode = g_skeleton_database->db_put(skel_record);

    g_skeleton_database->db_print(); // TODO remove later

    if (dbOpCode == RECORD_PUT_DUPLICATE) return 1; // warning


    // create MSG_REGISTER type msg
    // format: msg_len, msg_type, server_ip, server_port, fct_name_len, fct_name, num_args, argTypes
    if (assemble_msg(&msg, &msg_len, MSG_REGISTER, g_server_ip, g_server_port, name_len, name, num_args, argTypes) < 0) {
        fprintf(stderr, "ERROR creating registration request message\n");
        free(msg);
        return -1;
    }

    // send registration message to binder
    write_len = write_message(g_binder_fd,msg,msg_len);
    if ( write_len < msg_len ) {
        fprintf(stderr, "Error : couldn't send register request\n");
        return -1;
    }

    return 0;
} // rpcRegister

/**
 * Returns:
 * 0 normal termination
 * else abnormal termination
 */
int rpcExecute()
{

    int highest_fd, select_rv, client_fd;
    fd_set active_fds, read_fds;
    listen(g_server_fd, MAX_CLIENT_CONNECTIONS);

    struct sockaddr_in client_addr;
    unsigned int client_addr_len;

    char* connection_msg = NULL;
    ssize_t write_len;

    while (1) {

        read_fds = active_fds;
        select_rv = select(highest_fd, &read_fds, NULL, NULL, NULL);

        if (select_rv < 0) {
            fprintf(stderr, "ERROR on select: %s\n", strerror(errno));
            return -1;
        }

        // iterate through each socket
        for (unsigned int connection_fd = 0; connection_fd <= highest_fd; connection_fd++) {

            // this connection has no read requests
            if (!FD_ISSET(connection_fd, &read_fds)) continue;

            if (connection_fd == g_server_fd) {
                // this is the server itself
                client_fd = accept(g_server_fd, (struct sockaddr*) &client_addr, &client_addr_len);
                if (client_fd < 0) {
                    fprintf(stderr, "ERROR on accepting client: %s\n", strerror(errno));
                } else {
                    FD_SET(client_fd, &active_fds);
                }

            } else if (connection_fd == g_binder_fd) {
                // this is the binder connection

                if (read_message(connection_msg, connection_fd) < 0) {
                    fprintf(stderr, "ERROR reading from binder socket: %s\n", strerror(errno));
                    continue;
                }

                handle_binder_message(connection_msg, connection_fd);

            } else {
                // this is a client connection

                if (read_message(connection_msg, connection_fd) < 0) {
                    fprintf(stderr, "ERROR reading from client socket %d: %s\n", connection_fd, strerror(errno));
                    continue;
                }

                handle_client_message(connection_msg, connection_fd);

            }
        }

    }

    FD_ZERO(&active_fds);
    FD_ZERO(&read_fds);

    return 0;
}

int rpcTerminate()
{
    return -1;
}


int create_server_socket()
{

    struct sockaddr_in server_addr;
    unsigned int server_addr_len = sizeof(server_addr);

    // prep client listener socket info for binding
    memset(&server_addr, '0', server_addr_len);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // create client listener socket
    g_server_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_server_fd < 0) {
        fprintf(stderr, "ERROR creating client socket on server: %s\n", strerror(errno));
        return -1;
    }

    // bind client listener socket
    if ( bind(g_server_fd, (const struct sockaddr *)(&server_addr), server_addr_len) < 0 ) {
        fprintf(stderr,"ERROR binding client listener socket: %s\n", strerror(errno));
        return -1;
    }

    // set server ip and port to global variable
    g_server_port = server_addr.sin_port;
    if(get_ip_from_socket(&g_server_ip, g_server_fd) < 0) {
        return -1;
    }

    return 0;
} // create_server_socket


int connect_to_binder()
{

    struct sockaddr_in binder_addr;
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

    DEBUG("CONNECTED to binder: %s\n", binder_hostinfo->h_name);

    return 0;
} // connect_to_binder


void handle_binder_message(char* msg, unsigned int binder_fd)
{

} // handle_binder_message


void handle_client_message(char* msg, unsigned int client_fd)
{

} // handle_client_message

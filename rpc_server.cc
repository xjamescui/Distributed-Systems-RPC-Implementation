#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "debug.h"
#include "helper.h"
#include "rpc.h"
#include "SkeletonDatabase.h"
#include <pthread.h>
#include <list>


#define MAX_CLIENT_CONNECTIONS 100

using namespace std;


int create_server_socket();
int connect_server_to_binder();
int extract_registration_results(char *msg, int* result);
void* handle_client_message(void * hidden_args);

int g_binder_fd = -1;
int g_server_fd = -1;
unsigned int g_server_port, g_server_ip;
SkeletonDatabase* g_skeleton_database;

// threads
list<pthread_t> g_client_threads; // running client threads
pthread_mutex_t g_client_thread_lock = PTHREAD_MUTEX_INITIALIZER;



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
    if((binderOpCode = connect_server_to_binder()) < 0) return binderOpCode;

    // create server socket to listen to client
    if((serverOpCode = create_server_socket()) < 0) return serverOpCode;

    // initialize skeleton database
    g_skeleton_database = new SkeletonDatabase();

    return 0;
} // rpcInit


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

    // not connected to binder
    if (g_binder_fd < 0) return -1;

    unsigned int num_args, msg_len, name_len, write_len;
    char* msg = NULL;
    SKEL_RECORD skel_record;
    int dbOpCode;
    int register_result;

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
    free(msg);
    if ( write_len < msg_len ) {
        fprintf(stderr, "Error : couldn't send register request\n");
        return -1;
    }

    msg = NULL;
    read_message(&msg, g_binder_fd);
    extract_registration_results(msg,&register_result); // wait for binder response
    free(msg);

    return register_result;
} // rpcRegister


/**
 * Returns:
 * 0 normal termination
 * else abnormal termination
 */
int rpcExecute()
{

    int select_rv, client_fd;
    fd_set active_fds, read_fds;

    FD_ZERO(&active_fds);
    FD_ZERO(&read_fds);
    FD_SET(g_server_fd, &active_fds);
    FD_SET(g_binder_fd, &active_fds);

    listen(g_server_fd, MAX_CLIENT_CONNECTIONS);

    struct sockaddr_in client_addr;
    unsigned int client_addr_len;

    char* connection_msg = NULL;
    ssize_t write_len;
    bool running = true;

    while (running) {

        read_fds = active_fds;
        select_rv = select(FD_SETSIZE, &read_fds, NULL, NULL, NULL);

        if (select_rv < 0) {
            fprintf(stderr, "ERROR on select: %s\n", strerror(errno));
            return -1;
        }

        // iterate through each socket
        for (unsigned int connection_fd = 0; connection_fd <= FD_SETSIZE; connection_fd++) {

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

                char msg_type;
                unsigned int msg_len;
                if (read_message(&connection_msg, connection_fd) < 0) {
                    fprintf(stderr, "ERROR reading from binder socket: %s\n", strerror(errno));
                    continue;
                }

                extract_msg_len_type(&msg_len, &msg_type, connection_msg);

                if (msg_type == MSG_TERMINATE ) {
                    running = false;
                    break; // terminate
                }

            } else {
                // this is a client connection

                if (read_message(&connection_msg, connection_fd) < 0) {
                    fprintf(stderr, "ERROR reading from client socket %d: %s\n", connection_fd, strerror(errno));
                    continue;
                }

                // create a new thread to process requests from this client
                void** thread_args = new void*[2];
                thread_args[0] = (void *) connection_msg;
                thread_args[1] = (void *) new int(connection_fd);

                pthread_t client_thread;
                int bad_code = pthread_create(&client_thread, NULL, handle_client_message, (void *)thread_args);

                if (bad_code) {
                    // failed to create new thread
                    fprintf(stderr, "ERROR: failed to create a thread for client\n");
                    return -1;
                }

                pthread_mutex_lock( &g_client_thread_lock);
                // add this thread to list of all running threads
                g_client_threads.push_back(client_thread);
                pthread_mutex_unlock( &g_client_thread_lock);
            }
        } // for
    } // while

    FD_ZERO(&active_fds);
    FD_ZERO(&read_fds);

    // wait for all client threads to finish
    for (list<pthread_t>::iterator it = g_client_threads.begin(); it != g_client_threads.end(); ++it) {
        pthread_join((*it), NULL);
    }

    pthread_mutex_destroy(&g_client_thread_lock);

    close(g_binder_fd);
    close(g_server_fd);

    return 0;
}

int create_server_socket()
{

    struct sockaddr_in server_addr;
    unsigned int server_addr_len = sizeof(server_addr);
    int server_fd;

    // prep client listener socket info for binding
    memset(&server_addr, '0', server_addr_len);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // create client listener socket
    server_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd < 0) {
        fprintf(stderr, "ERROR creating client socket on server: %s\n", strerror(errno));
        return -1;
    }

    g_server_fd = server_fd;

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


int connect_server_to_binder()
{

    // declare sock vars
    int binder_fd;
    char* binder_address;
    char* binder_port_str;
    int binder_port;

    // get environment variables
    binder_address = getenv(BINDER_ADDRESS_STRING);
    binder_port_str = getenv(BINDER_PORT_STRING);
    if (binder_address == NULL || binder_port_str == NULL) {
        fprintf(stderr, "Error : connect_server_to_binder() BINDER_ADDRESS and/or BINDER_PORT not set\n");
        return -1;
    }
    binder_port = atoi(binder_port_str);

    // connect to binder
    if ( connect_to_hostname_port(&binder_fd, binder_address, binder_port) < 0 ) {
        fprintf(stderr, "Error : connect_server_to_binder() cannot connect to binder\n");
        return -1;
    }

    g_binder_fd = binder_fd;

    // DEBUG("SERVER CONNECTED to binder: %s\n", binder_hostinfo->h_name);

    return 0;
} // connect_server_to_binder



/**
 * Extract response from binder after a server method registration request is sent
 */
int extract_registration_results(char *msg, int* result)
{

    unsigned int msg_len;
    char msg_type;

    extract_msg_len_type(&msg_len, &msg_type, msg);
    switch (msg_type) {
    case MSG_REGISTER_SUCCESS:
    case MSG_REGISTER_FAILURE:
        // register success or failure
        if (extract_msg(msg, msg_len, msg_type, &result) < 0) {
            fprintf(stderr, "ERROR extracting msg\n");
            return -1;
        }
        return 0;
    default:
        return -1;
    }
} // extract_registration_results


/**
 * Thread task: handles client RPC requests and invoke the corresponding server methods
 * - return execution success/failure feedback to client
 */
void* handle_client_message(void * hidden_args)// char* msg, unsigned int client_fd
{

    // parse arguments
    void** argsArray = (void**) hidden_args;
    char* msg = (char *)(argsArray[0]);
    unsigned int client_fd = *((unsigned int*)(argsArray[1]));

    char msg_type;
    unsigned int msg_len, fct_name_len, arg_types_len;
    int* arg_types; // rpc function arg types
    void** args; // rpc function args
    char* fct_name = NULL;
    char* response_msg = NULL;
    skeleton target_method;
    int method_return_code;

    extract_msg_len_type(&msg_len, &msg_type, msg);
    if (msg_type == MSG_EXECUTE) {

        // extract msg
        if (extract_msg(msg, msg_len, msg_type, &fct_name_len, &fct_name, &arg_types_len, &arg_types, &args) < 0) {
            fprintf(stderr, "ERROR extracting msg\n");
        }

        // run requested server procedure
        g_skeleton_database->db_get(&target_method, fct_name, arg_types);

        // respond with execution results
        method_return_code = target_method(arg_types, args);

        if (method_return_code == 0) {
            // EXECUTE SUCCESS
            assemble_msg(&response_msg, &msg_len, MSG_EXECUTE_SUCCESS, fct_name_len, fct_name, arg_types_len, arg_types, args);
        } else if (method_return_code < 0) {
            // EXECUTE FAIL
            assemble_msg(&response_msg, &msg_len, MSG_EXECUTE_FAILURE, -1); // TODO: what should reason code be instead of -1?
        }

        // send response to client regarding RPC success/failure
        write_message(client_fd, response_msg, msg_len);
    }

} // handle_client_msg

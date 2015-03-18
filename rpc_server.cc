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

int g_binder_fd = RPC_SOCKET_UNINITIALIZED;
int g_server_fd = RPC_SOCKET_UNINITIALIZED;
fd_set g_active_fds;

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

    return RPC_SERVER_INIT_SUCCESS;
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
    if (g_binder_fd == RPC_SOCKET_UNINITIALIZED){
        fprintf(stderr, "ERROR: cannot execute rpcRegister because server is not connected to binder\n");
        return RPC_SOCKET_UNINITIALIZED; // error
    }

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

    if (dbOpCode == SKEL_RECORD_PUT_DUPLICATE) {
        fprintf(stdout, "%s already exists in the server database\n", name);
    } 

    // create MSG_REGISTER type msg
    // format: msg_len, msg_type, server_ip, server_port, fct_name_len, fct_name, num_args, argTypes
    if (assemble_msg(&msg, &msg_len, MSG_REGISTER, g_server_ip, g_server_port, name_len, name, num_args, argTypes) < 0) {
        fprintf(stderr, "ERROR creating registration request message\n");
        free(msg);
        return ASSEMBLE_MSG_FAIL;
    }

    // send registration message to binder
    write_len = write_message(g_binder_fd,msg,msg_len);
    free(msg);
    if ( write_len < msg_len ) {
        fprintf(stderr, "Error : couldn't send register request\n");
        return RPC_WRITE_TO_BINDER_FAIL;
    }

    msg = NULL;
    read_message(&msg, g_binder_fd);
    extract_registration_results(msg,&register_result); // wait for binder response
    free(msg);

    DEBUG("registration result is: %d\n", register_result);
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
    fd_set read_fds;

    FD_ZERO(&g_active_fds);
    FD_ZERO(&read_fds);
    FD_SET(g_server_fd, &g_active_fds);
    FD_SET(g_binder_fd, &g_active_fds);

    listen(g_server_fd, MAX_CLIENT_CONNECTIONS);

    char* connection_msg = NULL;
    bool running = true;

    while (running) {

        DEBUG("SELECT ... ");

        read_fds = g_active_fds;
        select_rv = select(FD_SETSIZE, &read_fds, NULL, NULL, NULL);

        DEBUG("SELECTED!");

        if (select_rv < 0) {
            fprintf(stderr, "ERROR on select: %s\n", strerror(errno));
            return RPC_CONNECTION_SELECT_FAIL;
        }

        // iterate through each socket
        for (unsigned int connection_fd = 0; connection_fd < FD_SETSIZE; connection_fd+=1 ) {

            // this connection has no read requests
            if (!FD_ISSET(connection_fd, &read_fds)) continue;

            if (connection_fd == (unsigned int)g_server_fd) {
                // this is the server itself
                client_fd = accept(g_server_fd, NULL, NULL);
                if (client_fd < 0) {
                    fprintf(stderr, "ERROR on accepting client: %s\n", strerror(errno));
                } else {
                    FD_SET(client_fd, &g_active_fds);
                }

            } else if (connection_fd == (unsigned int)g_binder_fd) {
                // this is the binder connection

                char msg_type;
                unsigned int msg_len;

                int read_len = read_message(&connection_msg, connection_fd);

                if (read_len == READ_MSG_ZERO_LENGTH) {
                  // binder is closed
                  close(connection_fd);
                  FD_CLR(connection_fd, &g_active_fds);
                  break;
                }

                if (read_len < 0) {
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
                    return PTHREAD_CREATE_FAIL;
                }

                pthread_mutex_lock( &g_client_thread_lock);
                // add this thread to list of all running threads
                g_client_threads.push_back(client_thread);
                pthread_mutex_unlock( &g_client_thread_lock);
            }
        } // for
    } // while

    FD_ZERO(&g_active_fds);
    FD_ZERO(&read_fds);

    // wait for all client threads to finish
    for (list<pthread_t>::iterator it = g_client_threads.begin(); it != g_client_threads.end(); ++it) {
        pthread_join((*it), NULL);
    }

    pthread_mutex_destroy(&g_client_thread_lock);

    close(g_binder_fd);
    close(g_server_fd);
    delete g_skeleton_database;

    return RPC_SERVER_SHUTDOWN_SUCCESS;
} // rpcExecute


/**
 * error codes:
 * RPC_SERVER_CREATE_SOCKET_FAIL
 * RPC_SERVER_BIND_SOCKET_FAIL
 * RPC_SERVER_GET_SOCK_NAME_FAIL
 * GET_IP_FROM_SOCKET_FAIL
 */
int create_server_socket()
{

    struct sockaddr_in server_addr;
    unsigned int server_addr_len = sizeof(server_addr);
    int server_fd;

    // prep client listener socket info for binding
    memset(&server_addr, 0, server_addr_len);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = 0;

    // create client listener socket
    server_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd < 0) {
        fprintf(stderr, "ERROR creating client socket on server: %s\n", strerror(errno));
        return RPC_SERVER_CREATE_SOCKET_FAIL; 
    }

    g_server_fd = server_fd;

    // bind client listener socket
    if ( bind(g_server_fd, (const struct sockaddr *)(&server_addr), server_addr_len) < 0 ) {
        fprintf(stderr,"ERROR binding client listener socket: %s\n", strerror(errno));
        return RPC_SERVER_BIND_SOCKET_FAIL;
    }

    // set server ip and port to global variable
    if (getsockname(g_server_fd, (struct sockaddr *)&server_addr, &server_addr_len) == -1){
        fprintf(stderr, "ERROR getsockname on server\n");
        return RPC_SERVER_GET_SOCK_NAME_FAIL;
    }

    g_server_port = server_addr.sin_port;

    if(get_ip_from_socket(&g_server_ip, g_server_fd) < 0) {
        return GET_IP_FROM_SOCKET_FAIL;
    }

    // prints out ip and port for debug purpose
    unsigned int hton_ip = htonl(g_server_ip);
    unsigned char ipb1 = (hton_ip >> 24) & 0xFF;
    unsigned char ipb2 = (hton_ip >> 16) & 0xFF;
    unsigned char ipb3 = (hton_ip >> 8) & 0xFF;
    unsigned char ipb4 = (hton_ip >> 0) & 0xFF;

    DEBUG("server addr:%u.%u.%u.%u",ipb1, ipb2, ipb3, ipb4);
    DEBUG("server port:%d",ntohs(g_server_port));
    DEBUG("(network)server port:%x",g_server_port);

    return RPC_SERVER_CREATE_SOCKET_SUCCESS;
} // create_server_socket


/**
 * error codes:
 * RPC_ENVR_VARIABLES_NOT_SET
 * RPC_CONNECT_TO_BINDER_FAIL
 */
int connect_server_to_binder()
{

    // declare sock vars
    int binder_fd;
    char* binder_address;
    char* binder_port_str;
    unsigned int binder_port;
    unsigned short binder_port_short;


    // get environment variables
    binder_address = getenv(BINDER_ADDRESS_STRING);
    binder_port_str = getenv(BINDER_PORT_STRING);
    if (binder_address == NULL || binder_port_str == NULL) {
        fprintf(stderr, "Error : connect_server_to_binder() BINDER_ADDRESS and/or BINDER_PORT not set\n");
        return RPC_ENVR_VARIABLES_NOT_SET;
    }
    binder_port = atoi(binder_port_str);
    binder_port_short = binder_port;
    binder_port_short = htons(binder_port_short);

    // connect to binder
    if ( connect_to_hostname_port(&binder_fd, binder_address, binder_port_short) < 0 ) {
        fprintf(stderr, "Error : connect_server_to_binder() cannot connect to binder\n");
        return RPC_CONNECT_TO_BINDER_FAIL;
    }

    g_binder_fd = binder_fd;

    // DEBUG("SERVER CONNECTED to binder: %s\n", binder_hostinfo->h_name);

    return 0;
} // connect_server_to_binder



/**
 * Extract response from binder after a server method registration request is sent
 *
 * error codes:
 * EXTRACT_MSG_FAIL
 * MSG_TYPE_NOT_SUPPORTED
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
                return EXTRACT_MSG_FAIL;
            }
            return 0;
        default:
            return MSG_TYPE_NOT_SUPPORTED;
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
    int* arg_types = NULL; // rpc function arg types
    void** args = NULL; // rpc function args
    char* fct_name = NULL;
    char* response_msg = NULL;
    skeleton target_method;
    int reason_code;

    extract_msg_len_type(&msg_len, &msg_type, msg);
    if (msg_type == MSG_EXECUTE) {

        // extract msg
        if (extract_msg(msg, msg_len, msg_type, &fct_name_len, &fct_name, &arg_types_len, &arg_types, &args) < 0) {
            fprintf(stderr, "ERROR extracting msg\n");
        }

        // run requested server procedure
        reason_code = g_skeleton_database->db_get(&target_method, fct_name, arg_types);

        if (reason_code == SKEL_RECORD_FOUND){
            // respond with execution results
            reason_code = target_method(arg_types, args);
        }

        if (reason_code == SKEL_EXEC_SUCCESS) {
            // EXECUTE SUCCESS
            assemble_msg(&response_msg, &msg_len, MSG_EXECUTE_SUCCESS, fct_name_len, fct_name, arg_types_len, arg_types, args);
        } else {
            // EXECUTE FAIL
            assemble_msg(&response_msg, &msg_len, MSG_EXECUTE_FAILURE, reason_code);
        }

        // send response to client regarding RPC success/failure
        write_message(client_fd, response_msg, msg_len);

    }

    FD_CLR(client_fd, &g_active_fds);
    close(client_fd);

    if (response_msg != NULL) free(response_msg);
    if (fct_name != NULL) free(fct_name);
    if (arg_types != NULL) free(arg_types);

    for ( unsigned int i = 0; i < arg_types_len; i += 1) {
        if (args[i] != NULL) free(args[i]);
    }

    if (args != NULL) free(args);

    return NULL;
} // handle_client_msg

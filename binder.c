// PF_INET, SOCK_STREAM, IPPROTO_TCP, AF_INET, INADDR_ANY
// socket, bind, listen, ntohs, htons, inet_ntoa
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include "stdbool.h"
#include <unistd.h>
#include <errno.h>

#include <string.h>

#include "rpc.h"
#include "debug.h"
#include "defines.h"
#include "helper.h"
#include "binder_database.h"

// prints BINDER_ADDRESS and BINDER_PORT to stdout
int print_address_and_port(int sock_fd, struct sockaddr_in sock_addr, unsigned int sock_addr_len);

// clean up and clean_and_exit
// ONLY CALL THIS IN MAIN THREAD
void clean_and_exit(int exit_code);

// handle REGISTER / LOC_REQUEST / TERMINATE
int handle_request(int connection_fd, fd_set *active_fds, fd_set *server_fds, bool *running);

int handle_register(int connection_fd, char *buffer, unsigned int buffer_len, fd_set *server_fds);
int handle_loc_request(int connection_fd, char *buffer, unsigned int buffer_len, fd_set *active_fds);
int handle_terminate(fd_set *active_fds, fd_set *server_fds);

/**
 * MAIN
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */


int main(int argc, char** argv)
{

    int binder_fd = 0;
    struct sockaddr_in binder_addr;
    unsigned int binder_addr_len = sizeof(binder_addr);

    // socket()
    if((binder_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        fprintf(stderr,"Error : socket() failed\n");
        clean_and_exit(1);
    }
    memset(&binder_addr, 0, binder_addr_len);
    binder_addr.sin_family = AF_INET;
    binder_addr.sin_addr.s_addr = INADDR_ANY;
    binder_addr.sin_port = 0;

    // bind()
    if ( bind(binder_fd, (const struct sockaddr *)(&binder_addr), binder_addr_len) < 0 ) {
        fprintf(stderr,"Error : bind() failed\n");
        clean_and_exit(1);
    }

    // listen()
    if ( listen(binder_fd,100) < 0 ) {
        fprintf(stderr,"Error : listen() failed\n");
        clean_and_exit(1);
    }

    // prints binder's BINDER_ADDRESS and BINDER_PORT
    if ( print_address_and_port(binder_fd,binder_addr,binder_addr_len) < 0 ) {
        clean_and_exit(1);
    }

    // initialize set of file descriptors
    fd_set temp_fds, active_fds, server_fds;
    FD_ZERO(&active_fds);
    FD_ZERO(&server_fds);
    FD_SET(binder_fd, &active_fds);

    int connection_fd;
    int exit_code = 0;
    bool running = true;
    while ( running ) {
        // block until one fd is ready
        temp_fds = active_fds;
        DEBUG("SELECT ...");
        if ( select(FD_SETSIZE, &temp_fds, NULL, NULL, NULL) < 0) {
            fprintf(stderr,"Error : select() failed\n");
            exit_code = 1;
            running = false;
            break;
        }
        DEBUG("SELECTED!");


        // check who's ready
        for ( int i = 0 ; i < FD_SETSIZE ; i += 1 ) {
            if ( FD_ISSET(i, &temp_fds) ) {
                if ( i == binder_fd) {
                    // there is a new connection request, gotta take it
                    connection_fd = accept(binder_fd,NULL,NULL);
                    if ( connection_fd < 0 ) {
                        fprintf(stderr,"Error : accept() failed\n");
                        exit_code = 1;
                        running = false;
                        break;
                    }
                    // add new connection to the active set
                    FD_SET(connection_fd,&active_fds);
                } else {
                    // a connected connection request arrived, gotta talk to it
                    if ( handle_request(i,&active_fds,&server_fds,&running) < 0 ) {
                        DEBUG("COULDN'T HANDLE REQUEST!!!");
                        exit_code = 1;
                    }
                    if ( running == false ) {
                        break;
                    }
                }
            } // if FD_ISSET
        } // for
    } // while

    DEBUG("SELECT LOOP OVER");

    clean_and_exit(exit_code);

    return 0;
}

void clean_and_exit(int exit_code)
{
    db_drop();
    DEBUG("BINDER EXITING WITH %d",exit_code);
    exit(exit_code);
}

/**
 * prints the ip and port
 * ex:
 * BINDER_ADDRESS 129.97.167.41
 * BINDER_PORT 10000
 */
int print_address_and_port(int sock_fd, struct sockaddr_in sock_addr, unsigned int sock_addr_len)
{
    struct hostent* host;

    // call gethostname to get the full host name
    char hostname[100] = { 0 };
    if ( gethostname(hostname,sizeof(hostname)) < 0 ) {
        fprintf(stderr,"Error : gethostname()\n");
        return -1;
    }
    if ( (host = gethostbyname(hostname)) == NULL ) {
        fprintf(stderr,"Error : gethostbyname() for %s\n",hostname);
        return -1;
    }
    // refresh sock_addr to output the port number
    if ( getsockname(sock_fd, (struct sockaddr *)&sock_addr, &sock_addr_len ) < 0 ) {
        fprintf(stderr,"Error : getsockname() failed\n");
        return -1;
    }

    unsigned int ipv4;
    unsigned char ipb1, ipb2, ipb3, ipb4;
    if ( get_ip_from_socket(&ipv4,sock_fd) < 0 ) {
        fprintf(stderr,"Error : couldn't get IP address for this server\n");
        return -1;
    }
    ipv4 = htonl(ipv4);
    ipb1 = (ipv4 >> 24) & 0xFF;
    ipb2 = (ipv4 >> 16) & 0xFF;
    ipb3 = (ipv4 >> 8) & 0xFF;
    ipb4 = (ipv4 >> 0) & 0xFF;

    fprintf(stdout,"BINDER_ADDRESS %u.%u.%u.%u\n",ipb1, ipb2, ipb3, ipb4);
    fprintf(stdout,"BINDER_PORT %d\n",ntohs(sock_addr.sin_port));
    fflush(stdout);
    return 0;
}


/**
 * HANDLING REQUESTS LOGIC
 *
 *
 *
 *
 *
 *
 */
// handle incoming request
int handle_request(int connection_fd, fd_set *active_fds, fd_set *server_fds, bool *running)
{
    ssize_t read_len;
    unsigned int msg_len;
    char msg_type;
    char* rw_buffer;
    unsigned int buffer_len;

    // read()
    rw_buffer = NULL;
    read_len = read_message(&rw_buffer,connection_fd);
    if ( read_len < 0 ) {
        fprintf(stderr,"Error : handle_request() couldn't read from socket\n");
        return -1;
    }

    // extract msg_len and msg_type
    extract_msg_len_type(&msg_len,&msg_type,rw_buffer);
    buffer_len = msg_len + 5;

    print_received_message(rw_buffer);

    // handle specific request according to its message code
    int return_code = 0;
    switch ( msg_type ) {
    case MSG_REGISTER: {
        if ( handle_register(connection_fd,rw_buffer,buffer_len,server_fds) < 0 ) {
            fprintf(stderr,"Error : handle_register() failed\n");
            return_code = -1;
        }
    }
    break;
    case MSG_LOC_REQUEST: {
        if ( handle_loc_request(connection_fd,rw_buffer,buffer_len,active_fds) < 0 ) {
            fprintf(stderr,"Error : handle_request() failed\n");
            return_code = -1;
        }
    }
    break;
    case MSG_TERMINATE: {
        free(rw_buffer);
        *running = false;
        if ( handle_terminate(active_fds,server_fds) < 0 ) {
            fprintf(stderr,"Error : handle_terminate() failed, terminate command not executed\n");
            return_code = -1;
        }
    }
    break;
    default:
        fprintf(stderr,"ERROR: binder does not handle this request type:%x\n",msg_type&0xff);
        return_code = -1;
    }


    return return_code;
}


/**
 * register
 *   - add server socket to server_fds
 * returns -1 if either read/write fails
 */
int is_valid_register(unsigned int ip, unsigned int port,
                      unsigned int fct_name_len, char* fct_name,
                      unsigned int arg_types_len, int* arg_types)
{
    // TODO: check if fct_name_len < 64
    // TODO: check the argTypes
    return 0;
}

int handle_register(int connection_fd, char *buffer, unsigned int buffer_len, fd_set *server_fds)
{

    // declare vars
    ssize_t write_len;

    // stuff that get extracted
    unsigned int server_ip;
    unsigned int server_port;
    unsigned int fct_name_len;
    char* fct_name = 0;
    unsigned int arg_types_len;
    int* arg_types = 0;

    // extract message
    extract_msg(buffer,buffer_len,MSG_REGISTER,
                &server_ip,&server_port,
                &fct_name_len,&fct_name,
                &arg_types_len,&arg_types);

    free(buffer);
    buffer = 0;

    // check if it's valid
    int is_valid = is_valid_register(server_ip,server_port,fct_name_len,fct_name,arg_types_len,arg_types);
    if ( is_valid < 0 ) {
        DEBUG("invalid register!! %d",is_valid);

        free(fct_name); // from extract_msg
        free(arg_types); // from extract_msg

        // send MSG_REGISTER_FAILURE
        assemble_msg(&buffer,&buffer_len,MSG_REGISTER_FAILURE,is_valid_register);
        write_len = write_message(connection_fd,buffer,buffer_len);
        free(buffer);
        if ( write_len < buffer_len ) {
            fprintf(stderr, "Error : couldn't send register request\n");
        }
        return -1;
    }

    // add message to the db
    SIGNATURE sig;
    sig.fct_name_len = fct_name_len;
    sig.fct_name = fct_name;
    sig.arg_types_len = arg_types_len;
    sig.arg_types = arg_types;

    HOST host;
    host.sock_fd = connection_fd;
    host.ip = server_ip;
    host.port = server_port;

    int put_result;
    put_result = db_put(host,sig);

    free(fct_name); // from extract_msg
    free(arg_types); // from extract_msg

    // send reply
    switch (put_result) {
    case BINDER_DB_PUT_SIGNATURE_SUCCESS: {
        assemble_msg(&buffer,&buffer_len,MSG_REGISTER_SUCCESS,MSG_REGISTER_SUCCESS_NO_ERRORS);
    }
    break;
    case BINDER_DB_PUT_SIGNATURE_DUPLICATE: {
        assemble_msg(&buffer,&buffer_len,MSG_REGISTER_SUCCESS,MSG_REGISTER_SUCCESS_OVERRIDE_PREVIOUS);
    }
    break;
    default:
        DEBUG("put result not handled yet %d",put_result);
        return -1;
    }

    write_len = write_message(connection_fd,buffer,buffer_len);
    free(buffer);
    if ( write_len < buffer_len ) {
        // should only get here if server terminated
        fprintf(stderr, "Error : couldn't send register request\n");
        return -1;
    }

    // add connection
    FD_SET(connection_fd,server_fds);
    return 0;

}

/**
 * loc request:
 *   - answers client with a ip and port
 *   - close client and remove it from active_fds
 * returns -1 if either fail to read/write
 */
int handle_loc_request(int connection_fd, char *buffer, unsigned int buffer_len, fd_set *active_fds)
{

    // read the message
    ssize_t write_len;

    // stuff that get extracted
    unsigned int fct_name_len;
    char* fct_name = 0;
    unsigned int arg_types_len;
    int* arg_types = 0;

    // extract message
    extract_msg(buffer,buffer_len,MSG_LOC_REQUEST,
                &fct_name_len,&fct_name,
                &arg_types_len,&arg_types);

    free(buffer);
    buffer = 0;

    SIGNATURE sig;
    sig.fct_name_len = fct_name_len;
    sig.fct_name = fct_name;
    sig.arg_types_len = arg_types_len;
    sig.arg_types = arg_types;

    // go through db to find function
    HOST host = { 0, 0, 0, 0 };

    int get_result;
    get_result = db_get(&host,sig);

    free(fct_name);
    free(arg_types);

    // make reply
    switch ( get_result ) {
    case BINDER_DB_GET_SIGNATURE_FOUND : {
        assemble_msg(&buffer,&buffer_len,MSG_LOC_SUCCESS,host.ip,host.port);
    }
    break;
    case BINDER_DB_GET_SIGNATURE_NOT_FOUND : {
        assemble_msg(&buffer,&buffer_len,MSG_LOC_FAILURE,MSG_LOC_FAILURE_SIGNATURE_NOT_FOUND);
    }
    break;
    case BINDER_DB_GET_SIGNATURE_HAS_NO_HOSTS : {
        assemble_msg(&buffer,&buffer_len,MSG_LOC_FAILURE,MSG_LOC_FAILURE_SIGNATURE_NO_HOSTS);
    }
    break;
    default:
        DEBUG("get result not handled yet %d",get_result);
        return -1;
    } // switch

    // send reply
    write_len = write_message(connection_fd,buffer,buffer_len);
    free(buffer);
    if ( write_len < buffer_len ) {
        fprintf(stderr, "handle_loc_request() write: is client still there?\n");
        return -1;
    }

    close(connection_fd);
    FD_CLR(connection_fd,active_fds);

    return 0;
}

/**
 * terminate:
 *    - send a message to every server
 *    - close each server socket
 *    - remove each socket from server_fds and active_fds
 */
int handle_terminate(fd_set *active_fds,fd_set *server_fds)
{

    char* rw_buffer = NULL;
    unsigned int rw_buffer_len;
    ssize_t write_len;

    assemble_msg(&rw_buffer,&rw_buffer_len,MSG_TERMINATE);

    // send a terminate request to all active servers in server_fds
    for ( int i = 0 ; i < FD_SETSIZE ; i += 1 ) {
        if ( FD_ISSET(i,server_fds) ) {
            write_len = write_message(i,rw_buffer,rw_buffer_len);
            if ( write_len < rw_buffer_len ) {
                fprintf(stderr,"Error : couldn't send terminate message to server at socket %d\n",i);
            }
            close(i);
            FD_CLR(i,server_fds);
            FD_CLR(i,active_fds);
        } else if ( FD_ISSET(i,active_fds) ) { // if it's client or binder itself
            close(i);
            FD_CLR(i,active_fds);
        }
    }
    return 0;
}

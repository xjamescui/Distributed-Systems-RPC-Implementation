

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
#include <unistd.h>
#include <errno.h>

#include <string.h>

#include "debug.h"
#include "defines.h"
#include "helper.h"

using namespace std;

#define DEBUG_BINDER_PORT 10000

// prints BINDER_ADDRESS and BINDER_PORT to stdout
int print_address_and_port(int sock_fd, struct sockaddr_in sock_addr, unsigned int sock_addr_len);

// clean up and clean_and_exit
// ONLY CALL THIS IN MAIN THREAD
void clean_and_exit(int exit_code);

// save a list of server_fd that are active



// handle REGISTER / LOC_REQUEST / TERMINATE
int handle_request(int connection_fd, fd_set *active_fds);
int handle_register(int msg_len) {
    // parse message
    // add message to the db
    return -1;
}
int handle_loc_request(int msg_len) {
    // go through queue to find function
    // ping server
    // return server address
    return -1;
}
int handle_terminate() {
    // send a terminate request to all active servers
    return -1;
}


int main(int argc, char** argv) {

    int binder_fd = 0;
    struct sockaddr_in binder_addr;
    unsigned int binder_addr_len = sizeof(binder_addr);

    // socket()
    if((binder_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        fprintf(stderr,"Error : socket() failed\n");
        clean_and_exit(1);
    }
    memset(&binder_addr, '0', binder_addr_len);
    binder_addr.sin_family = AF_INET;
    binder_addr.sin_addr.s_addr = INADDR_ANY;
#ifdef _ENABLE_DEBUG_
    binder_addr.sin_port = htons(DEBUG_BINDER_PORT);
#else
    binder_addr.sin_port = 0;
#endif

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
    fd_set read_fds, active_fds;
    FD_ZERO(&active_fds);
    FD_SET(binder_fd, &active_fds);

    int connection_fd;
    int exit_code = 0;
    bool running = true;
    while ( running ) {
        // block until one fd is ready
        read_fds = active_fds;
        if ( select(FD_SETSIZE, &read_fds, NULL, NULL, NULL) < 0) {
            fprintf(stderr,"Error : select() failed\n");
            exit_code = 1;
            running = false;
            break;
        }

        // check who's ready
        for ( int i = 0 ; i < FD_SETSIZE ; i += 1 ) {
            if ( FD_ISSET(i, &read_fds) ) {
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
                    if ( handle_request(i,&active_fds) < 0 ) {
                        DEBUG("COULDN'T HANDLE REQUEST!!!");
                        exit_code = 1;
                    }
                }
            } // if FD_ISSET
        } // for
    } // while

    // close binder port
    close(binder_fd);

    clean_and_exit(exit_code);
}

// handle incoming request
int handle_request(int connection_fd, fd_set *active_fds) {
    ssize_t read_len;
    unsigned int msg_len;
    char msg_type;
    char* rw_buffer;

    // read()
    rw_buffer = (char*)malloc(5);
    read_len = read(connection_fd,rw_buffer,5);

    if ( read_len <= 0 ) {
        fprintf(stderr,"Error : read() failed\n");
        free(rw_buffer);
        return -1;
    }
    if ( read_len != 5 ) {
        fprintf(stderr,"Error : read() didn't read size 5, msg not from protocol?\n");
        free(rw_buffer);
        return -1;
    }

    // extract msg_len and msg_type
    extract_msg_len_type(&msg_len,&msg_type,rw_buffer);
    free(rw_buffer);

    // handle specific request according to its message code
    int return_code = 0;
    switch ( msg_type ) {
    case MSG_REGISTER: {
        if ( handle_register(msg_len) < 0 ) {
            fprintf(stderr,"Error : handle_register() failed\n");
            return_code = -1;
        }
    } break;
    case MSG_LOC_REQUEST: {
        if ( handle_loc_request(msg_len) < 0 ) {
            fprintf(stderr,"Error : handle_request() failed\n");
            return_code = -1;
        }
    } break;
    case MSG_TERMINATE: {
        if ( handle_terminate() < 0 ) {
            fprintf(stderr,"Error : handle_terminate() failed\n");
            return_code = -1;
        }
    } break;
    default:
        fprintf(stderr,"ERROR: binder does not handle this request type:%x\n",msg_type&0xff);
        return_code = -1;
    }

    return return_code;
}



















void clean_and_exit(int exit_code) {
    exit(exit_code);
}




int print_address_and_port(int sock_fd, struct sockaddr_in sock_addr, unsigned int sock_addr_len){
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
    if ( getsockname(sock_fd, (struct sockaddr *)&sock_addr, &sock_addr_len ) < 0 ){
        fprintf(stderr,"Error : getsockname() failed\n");
        return -1;
    }

    unsigned int ipv4;
    unsigned char ipb1, ipb2, ipb3, ipb4;
    if ( get_ip_from_socket(&ipv4,sock_fd) < 0 ) {
        fprintf(stderr,"Error : couldn't get IP address for this server\n");
        return -1;
    }
    ipb1 = (ipv4 >> 24) & 0xFF;
    ipb2 = (ipv4 >> 16) & 0xFF;
    ipb3 = (ipv4 >> 8) & 0xFF;
    ipb4 = (ipv4 >> 0) & 0xFF;

    fprintf(stdout,"BINDER_ADDRESS %u.%u.%u.%u\n",ipb1, ipb2, ipb3, ipb4);
    fprintf(stdout,"BINDER_PORT %d\n",ntohs(sock_addr.sin_port));

    return 0;
}




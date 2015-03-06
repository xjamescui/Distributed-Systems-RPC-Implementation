

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
#include <string> // sizeof
#include <iostream>

#include "debug.h"
#include "helper.h"

using namespace std;

#define DEBUG_BINDER_PORT 10000

int main(int argc, char** argv) {

    int server_fd = 0;
    struct sockaddr_in serv_addr;
    unsigned int serv_addr_len = sizeof(serv_addr);
    struct hostent* host;

    // socket()
    if((server_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        cerr << "Error : socket() failed" << endl;
        return 1;
    }
    memset(&serv_addr, '0', serv_addr_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;

#ifdef _ENABLE_DEBUG_
    serv_addr.sin_port = htons(DEBUG_BINDER_PORT);
#else
    serv_addr.sin_port = 0;
#endif

    // bind()
    if ( bind(server_fd, (const struct sockaddr *)(&serv_addr), serv_addr_len) < 0 ) {
        cerr << "Error : bind() failed" << endl;
        return 1;
    }

    // call gethostname to get the full host name
    char hostname[100];
    if ( gethostname(hostname,sizeof(hostname)) < 0 ) {
        cerr << "Error : gethostname()" << endl;
        return 1;
    }
    if ( (host = gethostbyname(hostname)) == NULL ) {
        cerr << "Error : gethostbyname() for " << hostname << endl;
        return 1;
    }
    // refresh serv_addr to output the port number
    if ( getsockname(server_fd, (struct sockaddr *)&serv_addr, &serv_addr_len ) < 0 ){
        cerr << "Error : getsockname() failed" << endl;
        return 1;
    }

    unsigned int ipv4;
    unsigned char ipb1, ipb2, ipb3, ipb4;
    if ( get_ip_from_socket(ipv4,server_fd) < 0 ) {
        fprintf(stderr,"couldn't get IP address for this server\n");
        return 1;
    }
    ipb1 = (ipv4 >> 24) & 0xFF;
    ipb2 = (ipv4 >> 16) & 0xFF;
    ipb3 = (ipv4 >> 8) & 0xFF;
    ipb4 = (ipv4 >> 0) & 0xFF;

    fprintf(stdout,"SERVER_ADDRESS %u.%u.%u.%u\n",ipb1, ipb2, ipb3, ipb4);
    fprintf(stdout,"SERVER_PORT %d\n",ntohs(serv_addr.sin_port));









    return 0;
}






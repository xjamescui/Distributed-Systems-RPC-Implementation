

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
#include "helper.h"

using namespace std;

#define DEBUG_BINDER_PORT 10000

int main(int argc, char** argv) {

    int binder_fd = 0;
    struct sockaddr_in binder_addr;
    unsigned int binder_addr_len = sizeof(binder_addr);
    struct hostent* host;

    // socket()
    if((binder_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        fprintf(stderr,"Error : socket() failed\n");
        return 1;
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
        return 1;
    }

    // listen()
    if ( listen(binder_fd,100) < 0 ) {
        fprintf(stderr,"Error : listen() failed\n");
        return 1;
    }

    // prints binder's BINDER_ADDRESS and BINDER_PORT

    // call gethostname to get the full host name
    char hostname[100] = { 0 };
    if ( gethostname(hostname,sizeof(hostname)) < 0 ) {
        fprintf(stderr,"Error : gethostname()\n");
        return 1;
    }
    if ( (host = gethostbyname(hostname)) == NULL ) {
        fprintf(stderr,"Error : gethostbyname() for %s\n",hostname);
        return 1;
    }
    // refresh binder_addr to output the port number
    if ( getsockname(binder_fd, (struct sockaddr *)&binder_addr, &binder_addr_len ) < 0 ){
        fprintf(stderr,"Error : getsockname() failed\n");
        return 1;
    }

    unsigned int ipv4;
    unsigned char ipb1, ipb2, ipb3, ipb4;
    if ( get_ip_from_socket(ipv4,binder_fd) < 0 ) {
        fprintf(stderr,"Error : couldn't get IP address for this server\n");
        return 1;
    }
    ipb1 = (ipv4 >> 24) & 0xFF;
    ipb2 = (ipv4 >> 16) & 0xFF;
    ipb3 = (ipv4 >> 8) & 0xFF;
    ipb4 = (ipv4 >> 0) & 0xFF;

    fprintf(stdout,"BINDER_ADDRESS %u.%u.%u.%u\n",ipb1, ipb2, ipb3, ipb4);
    fprintf(stdout,"BINDER_PORT %d\n",ntohs(binder_addr.sin_port));










    return 0;
}






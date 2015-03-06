#include "helper.h"
#include "debug.h"
#include "rpc.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>  // inet_ntoa()
#include <net/if.h>     // IFNAMSIZ
#include <sys/ioctl.h>  // SIOCGIFADDR, ioctl()

/******************************************************************************
 * myread and mywrite
 *
 * 
 *****************************************************************************/
// div big chunk into smaller chunks and receive one by one
ssize_t myread(int fd, char* const buffer, const int buffer_len)
{
    int read_so_far = 0;
    ssize_t read_len;

    // clear buffer just in case
    memset(buffer,0,buffer_len);

    while ( read_so_far < buffer_len ) {
        read_len = read(fd, &buffer[read_so_far], MIN(MAX_RW_CHUNK_SIZE,buffer_len-read_so_far));
        if ( read_len < 0 ) {
            fprintf(stderr,"Error : Could only read %d of %d\n",read_so_far,buffer_len);
            return -1;
        }
        read_so_far += read_len;
    }
    DEBUG("myread: read %d of %d",read_so_far,buffer_len);
    return read_so_far;
}

// div big chunk into smaller chunks and send one by one
ssize_t mywrite(int fd, const char* const buffer, const int buffer_len)
{
    int write_so_far = 0;
    ssize_t write_len;

    while ( write_so_far < buffer_len ) {
        write_len = write(fd, &buffer[write_so_far], MIN(MAX_RW_CHUNK_SIZE,buffer_len-write_so_far));
        if ( write_len < 0 ) {
            fprintf(stderr,"Error : Could only write %d of %d\n",write_so_far,buffer_len);
            return -1;
        }
        write_so_far += write_len;
    }
    DEBUG("mywrite: sent %d of %d",write_so_far,buffer_len);
    return write_so_far;
}


/******************************************************************************
 * Type stuff
 *
 * 
 *****************************************************************************/
int compute_type_int(int &type_int, const bool is_input, const bool is_output, 
                      const char arg_type, const unsigned int arg_size ){
    type_int = 0;
    if ( is_input ) type_int = type_int | 1 << ARG_INPUT ;
    if ( is_output ) type_int = type_int | 1 << ARG_OUTPUT ;
    type_int = type_int | arg_type << 16 ;
    if ( ( arg_size & 0xFFFF0000 ) != 0 ) return -1;
    type_int = type_int | arg_size ;
    return 0;
}

int type_is_input(bool &is_input, const int type){
    is_input = 0x80000000 & type;
    return 0;
}

int type_is_output(bool &is_output, const int type){
    is_output = 0x40000000 & type;
    return 0;
}

int type_arg_type(char &arg_type, const int type){
    arg_type = (type >> 16) & 0x000000FF;
    return 0;
}

int type_arg_size(int &arg_size, const int type){
    arg_size = type & 0x0000FFFF;
    return 0;
}


/******************************************************************************
 * Returns IP given socket_fd
 *
 * 
 *****************************************************************************/
// http://stackoverflow.com/questions/4770985/something-like-startswithstr-a-str-b-in-c
bool starts_with(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}
// function to return eth0 or etho000 ip addr
int get_ip_from_socket(unsigned int &ip, int socket_fd) {
    unsigned int i;
    struct ifreq ifreqs[20];
    struct ifconf ic;

    ic.ifc_len = sizeof(ifreqs);
    ic.ifc_req = ifreqs;

    if (ioctl(socket_fd, SIOCGIFCONF, &ic) < 0) {
        return -1;
    }

    for (i = 0; i < ic.ifc_len/sizeof(struct ifreq); ++i) {
        if ( starts_with("eth0",ifreqs[i].ifr_name) ||
             starts_with("eth000",ifreqs[i].ifr_name) ){
            DEBUG("%s: %s", ifreqs[i].ifr_name,
                       inet_ntoa(((struct sockaddr_in*)&ifreqs[i].ifr_addr)->sin_addr));
            ip = htonl(((struct sockaddr_in*)&ifreqs[i].ifr_addr)->sin_addr.s_addr) ;
            return 0;
        }
    }
    return 0;
}














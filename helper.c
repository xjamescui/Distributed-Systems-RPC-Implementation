#include "helper.h"
#include "debug.h"
#include "defines.h"
#include "rpc.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */

#include <arpa/inet.h>  // inet_ntoa()
#include <net/if.h>     // IFNAMSIZ
#include <sys/ioctl.h>  // SIOCGIFADDR, ioctl()

/******************************************************************************
 * read and write large
 *
 * 
 *****************************************************************************/
// div big chunk into smaller chunks and receive one by one
ssize_t read_large(int fd, char* const buffer, const int buffer_len)
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
    DEBUG("read_large: read %d of %d",read_so_far,buffer_len);
    return read_so_far;
}

// div big chunk into smaller chunks and send one by one
ssize_t write_large(int fd, const char* const buffer, const int buffer_len)
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
    DEBUG("write_large: sent %d of %d",write_so_far,buffer_len);
    return write_so_far;
}


/******************************************************************************
 * Type stuff
 *
 * 
 *****************************************************************************/
int compute_type_int(int *type_int, const bool is_input, const bool is_output, 
                      const char arg_type, const unsigned int arg_size ){
    *type_int = 0;
    if ( is_input ) *type_int = *type_int | 1 << ARG_INPUT ;
    if ( is_output ) *type_int = *type_int | 1 << ARG_OUTPUT ;
    *type_int = *type_int | arg_type << 16 ;
    if ( ( arg_size & 0xFFFF0000 ) != 0 ) return -1;
    *type_int = *type_int | arg_size ;
    return 0;
}

int type_is_input(bool *is_input, const int type){
    *is_input = 0x80000000 & type;
    return 0;
}

int type_is_output(bool *is_output, const int type){
    *is_output = 0x40000000 & type;
    return 0;
}

int type_arg_type(char *arg_type, const int type){
    *arg_type = (type >> 16) & 0x000000FF;
    return 0;
}

int type_arg_size(int *arg_size, const int type){
    *arg_size = type & 0x0000FFFF;
    return 0;
}

/******************************************************************************
 * function for debugging
 *
 * 
 *****************************************************************************/
// prints the buffer in hex form
void print_buffer(const char* const buffer, int buffer_len) 
{
    int i;
    fprintf(stderr,"\tvv\tvv\tvv\tvv");
    for ( i = 0 ; i < buffer_len ; i += 1 ) {
        if ( i % 16 == 0 ) {
            fprintf(stderr,"\nD:%d",i);
        }
        fprintf(stderr,"\t%x",(char)buffer[i] & 0xff);
    }
    fprintf(stderr,"\n\t^^\t^^\t^^\t^^\t^^\n");
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
int get_ip_from_socket(unsigned int *ip, int socket_fd) {
    unsigned int i;
    struct ifreq ifreqs[20];
    struct ifconf ic;

    ic.ifc_len = sizeof(ifreqs);
    ic.ifc_req = ifreqs;

    if (ioctl(socket_fd, SIOCGIFCONF, &ic) < 0) {
        return -1;
    }

    for (i = 0; i < ic.ifc_len/sizeof(struct ifreq); ++i) {
        if ( starts_with("eth0",ifreqs[i].ifr_name) ){
            DEBUG("%s: %s", ifreqs[i].ifr_name,
                       inet_ntoa(((struct sockaddr_in*)&ifreqs[i].ifr_addr)->sin_addr));
            *ip = htonl(((struct sockaddr_in*)&ifreqs[i].ifr_addr)->sin_addr.s_addr) ;
            return 0;
        }
    }
    return 0;
}


/******************************************************************************
 * assemble buffer and read buffer
 *
 * will free and reallocate buffer
 *
 *****************************************************************************/
int assemble_msg(char** buffer, unsigned int *buffer_len, const char msg_type, ... ) {

    va_list vl;             // declare a list of args
    va_start(vl,msg_type);  // starts after msg_type

    switch ( msg_type ) {
    case MSG_REGISTER : {
        // len, type, ip, port, fct_name, num_arg_type, arg_type
        unsigned int ip = va_arg(vl,unsigned int);
        unsigned int port = va_arg(vl,unsigned int);
        char* fct_name = va_arg(vl,char*);
        unsigned int num_arg_type = va_arg(vl,unsigned int);
        int* arg_type = va_arg(vl,int*);

        // compute len + allocate right size
        *buffer_len = 0;
        *buffer_len += 4;                    // length = u_int
        *buffer_len += 1;                    // type = char
        *buffer_len += 4;                    // ip = 4 chars
        *buffer_len += 2;                    // port = 2 chars
        *buffer_len += FUNCTION_NAME_SIZE;   // name
        *buffer_len += 4;                    // num_arg_type = u_int
        *buffer_len += num_arg_type*4;       // arg_type = 4*num_arg_type

        *buffer = (char*)malloc(*buffer_len);
        memset(*buffer,'0',*buffer_len);

        // LLLLTSSSSPPFff......fNnnnTtttTtttTtttTttt
        // 01234567890123456789012345678901234567890123456789
        unsigned int msg_len = (*buffer_len) - 5;
        memcpy(&(*buffer)[0],&msg_len,4);           // set length
        memcpy(&(*buffer)[4],&msg_type,1);          // set type
        memcpy(&(*buffer)[5],&ip,4);                // set ip
        memcpy(&(*buffer)[9],&port,2);              // set port
        memcpy(&(*buffer)[11],fct_name,FUNCTION_NAME_SIZE); // set fct name
        memcpy(&(*buffer)[11+FUNCTION_NAME_SIZE],&num_arg_type,4); // set num_arg_type
        memcpy(&(*buffer)[15+FUNCTION_NAME_SIZE],arg_type,num_arg_type*4); // set argTypes

    } break;
    default:
        DEBUG("Warning: unknown msg_type");
        break;
    }

    va_end(vl);             // end list

    return 0;
}

int extract_msg_len_type(unsigned int *msg_len, char *msg_type, const char* const buffer) {
    // message: LLLLT[...]
    memcpy(msg_len,&buffer[0],4);
    memcpy(msg_type,&buffer[4],1);
    return 0;
}

int extract_msg(const char* const buffer, const unsigned int buffer_len, const char msg_type, ...) {
    if ( buffer == 0 ) {
        DEBUG("extract_msg() : cannot extract an empty buffer");
        return -1;
    }

    va_list vl;             // declare a list of args
    va_start(vl,msg_type);  // starts after msg_type

    switch ( msg_type ) {
    case MSG_REGISTER : {
        // len, type, ip, port, fct_name, num_arg_type, arg_type
        unsigned int *ip = va_arg(vl,unsigned int*);
        unsigned int *port = va_arg(vl,unsigned int*);
        char **fct_name = va_arg(vl,char**);
        unsigned int num_arg_type = 0;
        int **arg_type = va_arg(vl,int**);

        if ( *fct_name != NULL ) {
            free(*fct_name);
        }
        if ( *arg_type != NULL ) {
            free(*arg_type);
        }

        // LLLLTSSSSPPNNN......NaaaaTtttTtttTtttTttt
        // 01234567890123456789012345678901234567890123456789
        memcpy(ip,&buffer[5],4);           // extract ip
        memcpy(port,&buffer[9],2);         // extract port
        *fct_name = (char*)malloc(FUNCTION_NAME_SIZE); 
        memcpy(*fct_name,&buffer[11],FUNCTION_NAME_SIZE); // extract fct name
        memcpy(&num_arg_type,&buffer[11+FUNCTION_NAME_SIZE],4); // extract num_arg_type
        *arg_type = (int*)malloc(num_arg_type*4);
        memcpy(*arg_type,&buffer[15+FUNCTION_NAME_SIZE],num_arg_type*4); // extract argTypes

    } break;
    default:
        DEBUG("Warning: unknown msg_type");
        break;
    }

    va_end(vl);             // end list

    return 0;
}




















































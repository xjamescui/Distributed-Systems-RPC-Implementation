#include "helper.h"
#include "debug.h"
#include "defines.h"
#include "rpc.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */

#include <arpa/inet.h>  // inet_ntoa()
#include <net/if.h>     // IFNAMSIZ
#include <netdb.h>      // gethostbyname()
#include <sys/ioctl.h>  // SIOCGIFADDR, ioctl()

/******************************************************************************
 * Read and Write Message
 *****************************************************************************/


/**
 * Read message of buffer_len from socket fd into buffer
 */
ssize_t read_message(char** buffer, int socket_fd)
{
    unsigned int read_so_far = 0;
    ssize_t read_len;
    unsigned int msg_len;
    char msg_type;

    // get message len and msg type in first 5 bytes
    if (*buffer != NULL) free(*buffer);
    *buffer = (char*)malloc(5);
    read_so_far = read(socket_fd, *buffer ,5);

    if (read_so_far < 0) {
        fprintf(stderr,"Error reading first 5 bytes: %s\n" , strerror(errno));
        return -1;
    }

    // determine msg_len and msg type value
    memcpy(&msg_len, &(*buffer)[0], 4);
    memcpy(&msg_type, &(*buffer)[4], 1);

    // after determining msg length: put the message pieces back together
    msg_len = 5 + msg_len;
    free(*buffer);
    *buffer = (char*)malloc(msg_len);
    memset(*buffer,0,msg_len);
    memcpy(&(*buffer)[0], &msg_len, 4);
    memcpy(&(*buffer)[4], &msg_type, 1);

    while ( read_so_far < msg_len ) {
        read_len = read(socket_fd, &(*buffer)[read_so_far], MIN(MAX_RW_CHUNK_SIZE, msg_len-read_so_far));
        if ( read_len < 0 ) {
            fprintf(stderr,"Error: Could only read %d of %d\n",read_so_far,msg_len);
            return -1;
        }
        read_so_far += read_len;
    }
    DEBUG("read_message: read %d of %d", read_so_far, msg_len);
    return read_so_far;
}

/**
 * Write message(buffer) of buffer_len and send to socket fd
 */
ssize_t write_message(int fd, const char* const buffer, const unsigned int buffer_len)
{
    unsigned int write_so_far = 0;
    ssize_t write_len;

    while ( write_so_far < buffer_len ) {
        write_len = write(fd, &buffer[write_so_far], MIN(MAX_RW_CHUNK_SIZE,buffer_len-write_so_far));
        if ( write_len < 0 ) {
            fprintf(stderr,"Error : Could only write %d of %d\n",write_so_far,buffer_len);
            return -1;
        }
        write_so_far += write_len;
    }
    DEBUG("write_message: sent %d of %d",write_so_far,buffer_len);
    return write_so_far;
}


/******************************************************************************
 * Type stuff
 *
 *
 *****************************************************************************/
int compute_type_int(int *type_int, const bool is_input, const bool is_output,
                     const char arg_type, const unsigned int arg_size )
{
    *type_int = 0;
    if ( is_input ) *type_int = *type_int | 1 << ARG_INPUT ;
    if ( is_output ) *type_int = *type_int | 1 << ARG_OUTPUT ;
    *type_int = *type_int | arg_type << 16 ;
    if ( ( arg_size & 0xFFFF0000 ) != 0 ) return -1;
    *type_int = *type_int | arg_size ;
    return 0;
}

int type_is_input(bool *is_input, const int type)
{
    *is_input = ( type >> ARG_INPUT );
    return 0;
}

int type_is_output(bool *is_output, const int type)
{
    *is_output = ( type >> ARG_OUTPUT );
    return 0;
}

int type_arg_type(char *arg_type, const int type)
{
    *arg_type = (type >> 16) & 0x000000FF;
    return 0;
}

int type_arg_size(unsigned int *arg_size, const int type)
{
    *arg_size = type & 0x0000FFFF;
    return 0;
}

/**
 * calculate the total length of an argType
 * ex: double[4] will return 32
 */
int type_arg_total_length(const int type)
{

    char arg_type;
    unsigned int total_number;

    type_arg_type(&arg_type,type);
    type_arg_size(&total_number,type);

    unsigned int individual_size;
    switch ( arg_type ) {
    case ARG_CHAR :
        individual_size=sizeof(char) ;
        break;
    case ARG_SHORT :
        individual_size=sizeof(short int) ;
        break;
    case ARG_INT :
        individual_size=sizeof(int) ;
        break;
    case ARG_LONG :
        individual_size=sizeof(long int);
        break;
    case ARG_DOUBLE :
        individual_size=sizeof(double) ;
        break;
    case ARG_FLOAT :
        individual_size=sizeof(float) ;
        break;
    default:
        individual_size = 0;
        break;
    }

    if ( total_number == 0 ) {
        return individual_size;
    } else {
        return individual_size * total_number;
    }
}

bool type_is_array(const int type)
{
    unsigned int size;
    type_arg_size(&size,type);
    return size > 0 ? true : false;
}

/**
 * returns true/valid if and only if it satisfies the following
 * - must be either input or output or both
 * - type must be one of the defines one
 */
bool type_is_valid(const int type)
{

    // check if it's one of input/output (i.e. return false if it's not input and not output)
    if ( ( (type>>24) & 0xC0 ) == 0 ) {
        return false;
    }

    // check if the first 8 bits is xx00 0000
    if ( ( (type>>24) & 0x3F ) != 0 ) {
        return false;
    }

    // check if its arg_type is one of ARG_CHAR, ARG_SHORT, ARG_INT, ARG_LONG, ARG_DOUBLE, ARG_FLOAT
    char arg_type;
    type_arg_type(&arg_type,type);
    switch (arg_type) {
    case ARG_CHAR:
    case ARG_SHORT:
    case ARG_INT:
    case ARG_LONG:
    case ARG_DOUBLE:
    case ARG_FLOAT:
        break;
    default:
        return false;
    }
    return true;
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
    fprintf(stderr,"\n\t^^\t^^\t^^\t^^\n");
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
int get_ip_from_socket(unsigned int *ip, int socket_fd)
{
    unsigned int i;
    struct ifreq ifreqs[20];
    struct ifconf ic;

    ic.ifc_len = sizeof(ifreqs);
    ic.ifc_req = ifreqs;

    if (ioctl(socket_fd, SIOCGIFCONF, &ic) < 0) {
        return -1;
    }

    for (i = 0; i < ic.ifc_len/sizeof(struct ifreq); ++i) {
        if ( starts_with("eth0",ifreqs[i].ifr_name) ) {
            DEBUG("%s: %s", ifreqs[i].ifr_name,
                  inet_ntoa(((struct sockaddr_in*)&ifreqs[i].ifr_addr)->sin_addr));
            *ip = htonl(((struct sockaddr_in*)&ifreqs[i].ifr_addr)->sin_addr.s_addr) ;
            return 0;
        }
    }
    return 0;
}

/******************************************************************************
 * connect to given ip/hostname and port
 *
 *
 *****************************************************************************/
int connect_to_ip_port(int *out_sock_fd, const unsigned int ip, const unsigned int port )
{

    int sock_fd;
    struct sockaddr_in temp_socket_addr;
    unsigned int temp_socket_addr_len = sizeof(temp_socket_addr);

    // socket()
    if ( ( sock_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 ) {
        fprintf(stderr, "Error : connect_to_ip_port socket() failed: %s\n", strerror(errno));
        return -1;
    }

    // char* ip_addr = (char*)&ip;
    // for( int i = 0 ; i < 4 ; i += 1 ) {
    //     DEBUG("%u ",ip_addr[i] & 0xff);
    // }
    // DEBUG("\n");
    // DEBUG("previous line should start with 127\n");

    // prepare connect()
    memset(&temp_socket_addr, '\0', temp_socket_addr_len);
    temp_socket_addr.sin_family = AF_INET;
    memcpy(&temp_socket_addr.sin_addr, &ip, 4); // set ip
    temp_socket_addr.sin_port = htons(port); // set port

    // connect()
    if( connect( sock_fd , (struct sockaddr *)&temp_socket_addr , temp_socket_addr_len ) < 0) {
        fprintf(stderr,"Error : connect_to_ip_port connect() failed : %s\n",strerror(errno));
        return -1;
    }

    // set the return socket
    *out_sock_fd = sock_fd;
    return 0;
}


int connect_to_hostname_port(int *out_sock_fd, const char* const hostname, const unsigned int port )
{

    int sock_fd;
    struct sockaddr_in temp_socket_addr;
    unsigned int temp_socket_addr_len = sizeof(temp_socket_addr);
    struct hostent* temp_hostinfo;

    // socket()
    if ( ( sock_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 ) {
        fprintf(stderr, "Error : connect_to_hostname_port socket() failed: %s\n", strerror(errno));
        return -1;
    }

    temp_hostinfo = gethostbyname(hostname);

    memset(&temp_socket_addr, '0', temp_socket_addr_len);
    temp_socket_addr.sin_family = AF_INET;
    memcpy(&temp_socket_addr.sin_addr.s_addr, temp_hostinfo->h_addr , temp_hostinfo->h_length);
    temp_socket_addr.sin_port = htons(port);

    // connect()
    if( connect( sock_fd , (struct sockaddr *)&temp_socket_addr , temp_socket_addr_len ) < 0) {
        fprintf(stderr,"Error : connect_to_hostname_port connect() failed : %s\n",strerror(errno));
        return -1;
    }

    // set the return socket
    *out_sock_fd = sock_fd;
    return 0;
}

/******************************************************************************
 * Variable param function to assemble and extract messages
 *
 *
 *
 * assemble will free and alloc buffer again
 *
 * extract will free and allocate fct_name and arg_types
 *
 * return:
 * 0 = success
 * TODO: error is negative?
 *****************************************************************************/
int assemble_msg(char** buffer, unsigned int *buffer_len, const char msg_type, ... )
{

    va_list vl;             // declare a list of args
    va_start(vl,msg_type);  // starts after msg_type

    switch ( msg_type ) {

    /**
     * messages with just type + error code
     */
    case MSG_REGISTER_SUCCESS :
    case MSG_REGISTER_FAILURE :
    case MSG_LOC_FAILURE :
    case MSG_EXECUTE_FAILURE : {
        // length, type, result
        int register_result = va_arg(vl,int);

        *buffer_len = 0;
        *buffer_len += 4;                           // length = u_int
        *buffer_len += 1;                           // type = char
        *buffer_len += 4;                           // result = 4 chars

        *buffer = (char*)malloc(*buffer_len);
        memset(*buffer,'\0',*buffer_len);

        unsigned int msg_len = (*buffer_len) - 5;
        memcpy(&(*buffer)[0],&msg_len,4);           // set length
        memcpy(&(*buffer)[4],&msg_type,1);          // set type
        memcpy(&(*buffer)[5],&register_result,4);   // set result

    }
    break;

    /**
     * register
     */
    case MSG_REGISTER : {
        // len, type, ip, port, fct_name_len, fct_name, arg_types_len, arg_types
        unsigned int ip = va_arg(vl,unsigned int);
        unsigned int port = va_arg(vl,unsigned int) & 0x0000ffff;
        unsigned int fct_name_len = va_arg(vl,unsigned int);
        char* fct_name = va_arg(vl,char*);
        unsigned int arg_types_len = va_arg(vl,unsigned int);
        int* arg_types = va_arg(vl,int*);

        // compute len + allocate right size
        *buffer_len = 0;
        *buffer_len += 4;                           // length = u_int
        *buffer_len += 1;                           // type = char
        *buffer_len += 4;                           // ip = 4 chars
        *buffer_len += 2;                           // port = 2 chars
        *buffer_len += 4;                           // fct_name_len = int
        *buffer_len += fct_name_len;                // fct_name
        *buffer_len += 4;                           // arg_types_len = u_int
        *buffer_len += arg_types_len*4;             // arg_types = 4*arg_types_len

        if ( *buffer != NULL ) {
            free(*buffer);
        }
        *buffer = (char*)malloc(*buffer_len);
        memset(*buffer,'\0',*buffer_len);

        // LLLLTIIIIPPFFFFfff......fAAAAaaa.aaa.aaa.
        // 0123456789012345678901234567890123456789
        unsigned int msg_len = (*buffer_len) - 5;
        memcpy(&(*buffer)[0],&msg_len,4);           // set length
        memcpy(&(*buffer)[4],&msg_type,1);          // set type
        memcpy(&(*buffer)[5],&ip,4);                // set ip
        memcpy(&(*buffer)[9],&port,2);              // set port
        memcpy(&(*buffer)[11],&fct_name_len,4);     // set fct name length
        memcpy(&(*buffer)[15],fct_name,fct_name_len); // set fct name
        memcpy(&(*buffer)[15+fct_name_len],&arg_types_len,4); // set arg_types_len
        memcpy(&(*buffer)[19+fct_name_len],arg_types,arg_types_len*4); // set argTypes

    }
    break;

    /**
     * loc request
     */
    case MSG_LOC_REQUEST : {
        // length, type, fct_name_len, fct_name, arg_tyles_len, arg_types
        unsigned int fct_name_len = va_arg(vl,unsigned int);
        char* fct_name = va_arg(vl,char*);
        unsigned int arg_types_len = va_arg(vl,unsigned int);
        int* arg_types = va_arg(vl,int*);

        // compute len + allocate right size
        *buffer_len = 0;
        *buffer_len += 4;                           // length = u_int
        *buffer_len += 1;                           // type = char
        *buffer_len += 4;                           // fct_name_len = int
        *buffer_len += fct_name_len;                // fct_name
        *buffer_len += 4;                           // arg_types_len = u_int
        *buffer_len += arg_types_len*4;             // arg_types = 4*arg_types_len

        if ( *buffer != NULL ) {
            free(*buffer);
        }
        *buffer = (char*)malloc(*buffer_len);
        memset(*buffer,'\0',*buffer_len);

        // LLLLTFFFFfff......fAAAAaaa.aaa.aaa.aaa.a
        // 0123456789012345678901234567890123456789
        unsigned int msg_len = (*buffer_len) - 5;
        memcpy(&(*buffer)[0],&msg_len,4);           // set length
        memcpy(&(*buffer)[4],&msg_type,1);          // set type
        memcpy(&(*buffer)[5],&fct_name_len,4);      // set fct name length
        memcpy(&(*buffer)[9],fct_name,fct_name_len); // set fct name
        memcpy(&(*buffer)[9+fct_name_len],&arg_types_len,4); // set arg_types_len
        memcpy(&(*buffer)[13+fct_name_len],arg_types,arg_types_len*4); // set argTypes

    }
    break;
    case MSG_LOC_SUCCESS : {
        // length, type, ip, port
        unsigned int ip = va_arg(vl,unsigned int);
        unsigned int port = va_arg(vl,unsigned int) & 0x0000ffff;

        // compute len + allocate right size
        *buffer_len = 0;
        *buffer_len += 4;                           // length = u_int
        *buffer_len += 1;                           // type = char
        *buffer_len += 4;                           // ip = 4 chars
        *buffer_len += 2;                           // port = 2 chars

        if ( *buffer != NULL ) {
            free(*buffer);
        }
        *buffer = (char*)malloc(*buffer_len);
        memset(*buffer,'\0',*buffer_len);

        // LLLLTIIIIPP
        // 01234567890
        unsigned int msg_len = (*buffer_len) - 5;
        memcpy(&(*buffer)[0],&msg_len,4);           // set length
        memcpy(&(*buffer)[4],&msg_type,1);          // set type
        memcpy(&(*buffer)[5],&ip,4);                // set ip
        memcpy(&(*buffer)[9],&port,2);              // set port

    }
    break;

    /**
     * execute and execute success
     */
    case MSG_EXECUTE :
    case MSG_EXECUTE_SUCCESS : {
        // length, type, fct_name_len, fct_name, arg_types_len, arg_types, args
        unsigned int fct_name_len = va_arg(vl,unsigned int);
        char* fct_name = va_arg(vl,char*);
        unsigned int arg_types_len = va_arg(vl,unsigned int);
        int* arg_types = va_arg(vl,int*);
        void** args = va_arg(vl,void**);

        // compute len + allocate right size
        *buffer_len = 0;
        *buffer_len += 4;                           // length = u_int
        *buffer_len += 1;                           // type = char
        *buffer_len += 4;
        *buffer_len += fct_name_len;
        *buffer_len += 4;                           // arg_types_len = u_int
        *buffer_len += arg_types_len*4;             // arg_types = 4*arg_types_len
        for ( unsigned int i = 0 ; i < arg_types_len ; i += 1 ) {
            *buffer_len += type_arg_total_length(arg_types[i]);   // dynamically compute length
        }

        if ( *buffer != NULL ) {
            free(*buffer);
        }
        *buffer = (char*)malloc(*buffer_len);
        memset(*buffer,'\0',*buffer_len);

        // assume arg1 = 2 int, arg2 = double, arg3 = double
        // LLLLTFFFFfff......fAAAAa...a...a...i...i...d.......d.......
        // 012345678901234567890123456789012345678901234567890123456789
        unsigned int msg_len = (*buffer_len) - 5;
        memcpy(&(*buffer)[0],&msg_len,4);           // set length
        memcpy(&(*buffer)[4],&msg_type,1);          // set type
        memcpy(&(*buffer)[5],&fct_name_len,4);      // set fct name length
        memcpy(&(*buffer)[9],fct_name,fct_name_len); // set fct name
        memcpy(&(*buffer)[9+fct_name_len],&arg_types_len,4); // set arg_types_len
        memcpy(&(*buffer)[13+fct_name_len],arg_types,arg_types_len*4); // set argTypes
        unsigned int buffer_current_index = 4+1+4+fct_name_len+4+arg_types_len*4;
        unsigned int single_arg_type_len = 0;
        for ( unsigned int i = 0 ; i < arg_types_len ; i += 1 ) {
            single_arg_type_len = type_arg_total_length(arg_types[i]);
            memcpy(&(*buffer)[buffer_current_index],args[i],single_arg_type_len);
            buffer_current_index += single_arg_type_len;
        }

    }
    break;


    /**
     * terminate
     */
    case MSG_TERMINATE : {
        *buffer_len = 0;
        *buffer_len += 4;                           // length = u_int
        *buffer_len += 1;                           // type = char

        *buffer = (char*)malloc(*buffer_len);
        memset(*buffer,'0',*buffer_len);

        unsigned int msg_len = (*buffer_len) - 5;
        memcpy(&(*buffer)[0],&msg_len,4);           // set length
        memcpy(&(*buffer)[4],&msg_type,1);          // set type
    }
    break;

    /**
     * unknown?????
     */
    default:
        DEBUG("Warning: unknown msg_type: %x",msg_type);
        break;
    }

    va_end(vl);             // end list

    return 0;
}


int extract_msg_len_type(unsigned int *msg_len, char *msg_type, const char* const buffer)
{
    // message: LLLLT[...]
    memcpy(msg_len,&buffer[0],4);
    memcpy(msg_type,&buffer[4],1);
    return 0;
}

int extract_msg(const char* const buffer, const unsigned int buffer_len, const char msg_type, ...)
{
    if ( buffer == 0 ) {
        DEBUG("extract_msg() : cannot extract an empty buffer");
        return -1;
    }

    va_list vl;             // declare a list of args
    va_start(vl,msg_type);  // starts after msg_type

    switch ( msg_type ) {

    /**
     * messages with just type + error code
     */
    case MSG_REGISTER_SUCCESS :
    case MSG_REGISTER_FAILURE :
    case MSG_LOC_FAILURE :
    case MSG_EXECUTE_FAILURE : {
        // length, type, result
        int *register_result = va_arg(vl,int*);

        memcpy(register_result,&buffer[5],4);           // extract result

    }
    break;

    /**
     * register
     */
    case MSG_REGISTER : {
        // len, type, ip, port, fct_name_len, fct_name, arg_types_len, arg_types
        unsigned int *ip = va_arg(vl,unsigned int*);
        unsigned int *port = va_arg(vl,unsigned int*);
        unsigned int *fct_name_len = va_arg(vl,unsigned int*);
        char **fct_name = va_arg(vl,char**);
        unsigned int *arg_types_len = va_arg(vl,unsigned int*);
        int **arg_types = va_arg(vl,int**);

        *port = 0;
        if ( *fct_name != NULL ) {
            free(*fct_name);
        }
        if ( *arg_types != NULL ) {
            free(*arg_types);
        }

        // LLLLTIIIIPPFFFFfff......fAAAAaaa.aaa.aaa.
        // 0123456789012345678901234567890123456789
        memcpy(ip,&buffer[5],4);                        // extract ip
        memcpy(port,&buffer[9],2);                      // extract port
        memcpy(fct_name_len,&buffer[11],4);             // extract fct name len
        *fct_name = (char*)malloc(*fct_name_len);
        memcpy(*fct_name,&buffer[15],*fct_name_len);    // extract fct name
        memcpy(arg_types_len,&buffer[15+(*fct_name_len)],4); // extract arg_types_len
        *arg_types = (int*)malloc(((*arg_types_len)+1)*4);
        memset(*arg_types,0,((*arg_types_len)+1)*4);
        memcpy(*arg_types,&buffer[19+(*fct_name_len)],(*arg_types_len)*4); // extract argTypes

    }
    break;

    /**
     * loc request
     */
    case MSG_LOC_REQUEST : {
        // len, type, fct_name_len, fct_name, arg_types_len, arg_types
        unsigned int *fct_name_len = va_arg(vl,unsigned int*);
        char **fct_name = va_arg(vl,char**);
        unsigned int *arg_types_len = va_arg(vl,unsigned int*);
        int **arg_types = va_arg(vl,int**);

        if ( *fct_name != NULL ) {
            free(*fct_name);
        }
        if ( *arg_types != NULL ) {
            free(*arg_types);
        }

        // LLLLTFFFFfff......fAAAAaaa.aaa.aaa.
        // 01234567890123456789012345678901234
        memcpy(fct_name_len,&buffer[5],4);              // extract fct name len
        *fct_name = (char*)malloc(*fct_name_len);
        memcpy(*fct_name,&buffer[9],*fct_name_len);     // extract fct name
        memcpy(arg_types_len,&buffer[9+(*fct_name_len)],4);  // extract arg_types_len
        *arg_types = (int*)malloc(((*arg_types_len)+1)*4); // alloc one more
        memset(*arg_types,0,((*arg_types_len)+1)*4);
        memcpy(*arg_types,&buffer[13+(*fct_name_len)],(*arg_types_len)*4); // extract argTypes

    }
    break;
    case MSG_LOC_SUCCESS : {
        // len, type, ip, port, fct_name_len, fct_name, arg_types_len, arg_types
        unsigned int *ip = va_arg(vl,unsigned int*);
        unsigned int *port = va_arg(vl,unsigned int*);

        *port = 0;

        // LLLLTIIIIPP
        // 01234567890
        memcpy(ip,&buffer[5],4);                        // extract ip
        memcpy(port,&buffer[9],2);                      // extract port

    }
    break;

    /**
     * execute and execute success
     */
    case MSG_EXECUTE :
    case MSG_EXECUTE_SUCCESS : {
        // length, type, fct_name_len, fct_name, arg_types_len, arg_types, args
        unsigned int *fct_name_len = va_arg(vl,unsigned int*);
        char **fct_name = va_arg(vl,char**);
        unsigned int *arg_types_len = va_arg(vl,unsigned int*);
        int **arg_types = va_arg(vl,int**);
        void ***args = va_arg(vl,void***);

        if ( *fct_name != NULL ) {
            free(*fct_name);
        }
        if ( *arg_types != NULL ) {
            free(*arg_types);
        }
        if ( *args != NULL ) {
            free(*args);
        }

        // LLLLTFFFFfff......fAAAAaaa.aaa.aaa.
        // 01234567890123456789012345678901234
        memcpy(fct_name_len,&buffer[5],4);              // extract fct name len
        *fct_name = (char*)malloc(*fct_name_len);
        memcpy(*fct_name,&buffer[9],*fct_name_len);     // extract fct name
        memcpy(arg_types_len,&buffer[9+(*fct_name_len)],4);  // extract arg_types_len
        *arg_types = (int*)malloc((*arg_types_len+1)*4); // alloc one more
        memset(*arg_types,0,(*arg_types_len+1)*4);
        memcpy(*arg_types,&buffer[13+(*fct_name_len)],(*arg_types_len)*4); // extract argTypes
        *args = (void**)malloc((*arg_types_len)*sizeof(void*));

        // extract each single arg
        unsigned int buffer_current_index = 4+1+4+(*fct_name_len)+4+(*arg_types_len)*4;
        unsigned int single_arg_type_len = 0;
        for ( unsigned int i = 0 ; i < *arg_types_len ; i += 1 ) {
            single_arg_type_len = type_arg_total_length((*arg_types)[i]);
            // DEBUG("type total len:%d , buffer_current_index:%d",single_arg_type_len,buffer_current_index);
            (*args)[i] = malloc(single_arg_type_len);
            memcpy((*args)[i],&buffer[buffer_current_index],single_arg_type_len);
            buffer_current_index += single_arg_type_len;
        }

    }
    break;

    /**
     * unknown?????
     */
    default:
        DEBUG("Warning: unknown msg_type: %x",msg_type);
        break;
    }

    va_end(vl);             // end list

    return 0;
}

/**
 * Get the length of argTypes array, knowing that the last element is 0
 */
unsigned int arg_types_length(int* argTypes)
{
    unsigned int len = 0;
    while (*argTypes != 0) {
        argTypes += 1;
        len += 1;
    }
    return len;
}

/**
 * step by step copy void** args into another void** args
 * (not changing pointers, assuming it's already allocated)
 */
int copy_args_step_by_step(int const *arg_types, void** const to_args, void const *const *const from_args)
{
    unsigned int arg_types_len = arg_types_length((int*)arg_types);


    // DEBUG("#types=%d",arg_types_len);

    int single_arg_total_len = 0;
    for ( unsigned int i = 0 ; i < arg_types_len ; i += 1 ) {
        single_arg_total_len = type_arg_total_length(arg_types[i]);
        if ( single_arg_total_len == 0 ) return -1;
        memcpy(to_args[i],from_args[i],single_arg_total_len);
    }

    return 0;
}


/**
 * a debug function to print out the message
 */
void print_received_message(char const *const buffer) {

#ifdef _ENABLE_DEBUG_
    char to_print[1000] = { 0 };

    unsigned int buffer_len;
    unsigned int msg_len;
    char msg_type;

    // stuff to get extracted

    extract_msg_len_type(&msg_len,&msg_type,buffer);
    buffer_len = msg_len + 5;


    sprintf(to_print,"request:0x%x ",(unsigned char)(msg_type&0xff));

    switch ( msg_type ) {
    case MSG_REGISTER: {
        unsigned int ip;
        unsigned int port;
        unsigned int fct_name_len;
        char* fct_name = NULL;
        unsigned int arg_types_len;
        int* arg_types = NULL;

        extract_msg(buffer,buffer_len,msg_type,
                    &ip,&port,&fct_name_len,&fct_name,&arg_types_len,&arg_types);

        unsigned char ipb1, ipb2, ipb3, ipb4;
        ipb1 = (ip >> 24) & 0xFF;
        ipb2 = (ip >> 16) & 0xFF;
        ipb3 = (ip >> 8) & 0xFF;
        ipb4 = (ip >> 0) & 0xFF;
        sprintf(to_print,"%s%u.%u.%u.%u:%u %s(",to_print,ipb1, ipb2, ipb3, ipb4, port, fct_name);

        char single_arg_type_type;
        unsigned int single_arg_type_size;

        for ( unsigned int i = 0 ; i < arg_types_len ; i += 1 ) {
            if ( i != 0 ) sprintf(to_print,"%s, ",to_print);

            type_arg_type(&single_arg_type_type,arg_types[i]);
            switch(single_arg_type_type) {
            case ARG_CHAR : { sprintf(to_print,"%schar",to_print); } break;
            case ARG_SHORT : { sprintf(to_print,"%sshort",to_print); } break;
            case ARG_INT : { sprintf(to_print,"%sint",to_print); } break;
            case ARG_LONG : { sprintf(to_print,"%slong",to_print); } break;
            case ARG_DOUBLE : { sprintf(to_print,"%sdouble",to_print); } break;
            case ARG_FLOAT : { sprintf(to_print,"%sfloat",to_print); } break;
            }

            type_arg_size(&single_arg_type_size,arg_types[i]);
            if ( type_is_array(arg_types[i]) ) {
                sprintf(to_print,"%s[%d]",to_print,single_arg_type_size);
            }
        }
        sprintf(to_print,"%s)",to_print);


        free(fct_name);
        free(arg_types);
        
    } break;
    case MSG_LOC_REQUEST: {
        unsigned int fct_name_len;
        char* fct_name = NULL;
        unsigned int arg_types_len;
        int* arg_types = NULL;

        extract_msg(buffer,buffer_len,msg_type,
                    &fct_name_len,&fct_name,&arg_types_len,&arg_types);

        sprintf(to_print,"%s %s(",to_print,fct_name);

        char single_arg_type_type;
        unsigned int single_arg_type_size;

        for ( unsigned int i = 0 ; i < arg_types_len ; i += 1 ) {
            if ( i != 0 ) sprintf(to_print,"%s, ",to_print);

            type_arg_type(&single_arg_type_type,arg_types[i]);
            switch(single_arg_type_type) {
            case ARG_CHAR : { sprintf(to_print,"%schar",to_print); } break;
            case ARG_SHORT : { sprintf(to_print,"%sshort",to_print); } break;
            case ARG_INT : { sprintf(to_print,"%sint",to_print); } break;
            case ARG_LONG : { sprintf(to_print,"%slong",to_print); } break;
            case ARG_DOUBLE : { sprintf(to_print,"%sdouble",to_print); } break;
            case ARG_FLOAT : { sprintf(to_print,"%sfloat",to_print); } break;
            }

            type_arg_size(&single_arg_type_size,arg_types[i]);
            if ( type_is_array(arg_types[i]) ) {
                sprintf(to_print,"%s[%d]",to_print,single_arg_type_size);
            }
        }
        sprintf(to_print,"%s)",to_print);
        free(fct_name);
        free(arg_types);

    } break;
    case MSG_TERMINATE: {
        sprintf(to_print,"(MSG_TERMINATE)");
    } break;
    default:
        DEBUG("print_received_message can't print unknown type");
        return;
    }
    
    DEBUG("%s",to_print);
#endif
}

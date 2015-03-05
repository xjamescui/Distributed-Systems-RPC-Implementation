#include "helper.h"
#include "debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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
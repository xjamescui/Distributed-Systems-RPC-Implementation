#ifndef HELPER_H
#define HELPER_H

#include <string.h>
#include <unistd.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define MAX_RW_CHUNK_SIZE 16348

/** read() that can take a large buffer_len
 *  returns # of chars read if successful
 *          -1 if fail
 */
extern ssize_t myread(int fd, char* const buffer, const unsigned int buffer_len);

/** write() that can take a large buffer_len
 *  returns # of chars read if successful
 *          -1 if fail
 */
extern ssize_t mywrite(int fd, const char* const buffer, const unsigned int buffer_len);







#endif // HELPER_H
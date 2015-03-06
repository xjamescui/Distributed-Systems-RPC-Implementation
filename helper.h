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
ssize_t read_large(int fd, char* const buffer, const unsigned int buffer_len);

/** write() that can take a large buffer_len
 *  returns # of chars read if successful
 *          -1 if fail
 */
ssize_t write_large(int fd, const char* const buffer, const unsigned int buffer_len);


/**
 * returns the type as specified in assignment
 * 1st byte: XY00 0000  - X: is input, Y: is output
 * 2nd byte: 0000 0ZZZ  - Z: type
 * 3rd byte: SSSS SSSS  - S: size
 * 4th byte: SSSS SSSS  - S=0 if scalar
 *        
 * returns 0 if successful
 *         -1 if fails
 */
int compute_type_int(int &type_int, const bool is_input, const bool is_output, 
                      const unsigned int arg_type, const unsigned int arg_size );
int type_is_input(bool &is_input, const int type);
int type_is_output(bool &is_output, const int type);
int type_arg_type(char &arg_type, const int type);
int type_arg_size(int &arg_size, const int type);


/**
 * returns IP given a socket_fd
 */
int get_ip_from_socket(unsigned int &ip, int socket_fd);



#endif // HELPER_H
#ifndef HELPER_H
#define HELPER_H

#include <unistd.h> // ssize_t

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

/** read() that can take a large buffer_len
 *  returns # of chars read if successful
 *          -1 if fail
 */
ssize_t read_message(char** buffer, int socket_fd);

/** write() that can take a large buffer_len
 *  returns # of chars read if successful
 *          -1 if fail
 */
ssize_t write_message(int fd, const char* const buffer, const unsigned int buffer_len);


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
int compute_type_int(int *type_int, const bool is_input, const bool is_output, 
                      const unsigned int arg_type, const unsigned int arg_size );
int type_is_input(bool *is_input, const int type);
int type_is_output(bool *is_output, const int type);
int type_arg_type(char *arg_type, const int type);
int type_arg_size(unsigned int *arg_size, const int type);

int type_arg_total_length(const int type);
bool type_is_valid(const int type);
bool type_is_array(const int type);


/**
 * helper function for debugging
 *
 */
void print_buffer(const char* const buffer, int buffer_len);

unsigned int arg_types_length(int* argTypes);


/**
 * returns IP given a socket_fd
 *
 */
int get_ip_from_socket(unsigned int *ip, int socket_fd);

/**
 * connect to a given IP and port
 *
 */
int connect_to_ip_port(int *out_sock_fd, const unsigned int ip, const unsigned int port );

/**
 * function to assemble or read the message described in protocol section 5
 * buffer = length(4) + type(1) + message(m)
 */
int assemble_msg(char** buffer, unsigned int *buffer_len, const char msg_type, ... );
int extract_msg_len_type(unsigned int *msg_len, char *msg_type, const char* const buffer);
int extract_msg(const char* const buffer, const unsigned int buffer_len, const char msg_type, ...);




#endif // HELPER_H

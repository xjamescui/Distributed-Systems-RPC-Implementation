#include "rpc.h"
#include "helper.h"
#include "defines.h"
#include "debug.h"

#include <stdlib.h>

#include <assert.h>
#include <string.h>

void assertTypeEquals(int type, bool is_in, bool is_out, char arg_type, int arg_size) {

    bool temp_is_in;
    bool temp_is_out;
    char temp_arg_type;
    int temp_arg_size;

    assert( type_is_input(&temp_is_in,type) >= 0 );
    assert( type_is_output(&temp_is_out,type) >= 0 );
    assert( type_arg_type(&temp_arg_type,type) >= 0 );
    assert( type_arg_size(&temp_arg_size,type) >= 0 );

    assert( temp_is_in == is_in );
    assert( temp_is_out == is_out );
    assert( temp_arg_type == arg_type );
    assert( temp_arg_size == arg_size );
}


int main() {
    //            AAAAAAAABBBBBBBBCCCCCCCCDDDDDDDD
    int type1 = 0b11000000000000110000000000000111;

    assertTypeEquals(type1,true,true,3,7);

    // test assemble message
    char* buffer = 0;
    unsigned int buffer_len;
    int* argTypes = (int*) malloc(2*sizeof(int));
    argTypes[0] = type1;
    argTypes[1] = type1;

    char fct_name[] = "abcd 1234 1234";
    assemble_msg(&buffer,&buffer_len,
        MSG_REGISTER,0x12121212,0x3344,fct_name,2,argTypes);

    assert(buffer_len == (4 + 1 + 4 + 2 + FUNCTION_NAME_SIZE + 4 + 8 ) );

    printf("buffer_len=%d\n",buffer_len);
    print_buffer(buffer,buffer_len);

    char msg_type;
    unsigned int msg_len;
    extract_msg_len_type(&msg_len,&msg_type,buffer);
    assert(msg_len == buffer_len - 5);
    assert(msg_type == MSG_REGISTER);
    printf("msg_len=%d\n",msg_len);

    unsigned int ip, port;
    char* fct_name2 = 0;
    int* argTypes2 = 0;

    extract_msg(buffer,buffer_len,
        MSG_REGISTER,&ip,&port,&fct_name2,&argTypes2);

    assert(ip == 0x12121212);
    assert(port == 0x3344);
    assert(strcmp(fct_name2,fct_name) == 0);
    assert(argTypes2 != NULL);
    assert(argTypes2[0] == type1);
    assert(argTypes2[1] == type1);



    return 0;
}

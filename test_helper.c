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

int mystrcmp(const char* const a, const char* const b, const unsigned int length) { // return 0 if same, or 1 if not same
    for ( int i = 0 ; i < length ; i += 1 ) {
        if ( a[i] != b[i] ) {
            return 1;
        }
    }
    return 0;
}


void test_short_messages() {

    char* buffer = 0;
    unsigned int buffer_len;
    int result = -2;
    unsigned int msg_len;
    char msg_type;

    int result2;

    // terminate
    // printf("test terminate\n");
    assemble_msg(&buffer,&buffer_len,MSG_TERMINATE);
    // print_buffer(buffer,buffer_len);
    extract_msg_len_type(&msg_len,&msg_type,buffer);
    assert(msg_len == buffer_len - 5);
    assert(msg_type == MSG_TERMINATE);
    free(buffer);
    buffer = 0;

    // register success
    // printf("test register success\n");
    assemble_msg(&buffer,&buffer_len,MSG_REGISTER_SUCCESS,result);
    // print_buffer(buffer,buffer_len);
    extract_msg_len_type(&msg_len,&msg_type,buffer);
    assert(msg_len == buffer_len - 5);
    assert(msg_type == MSG_REGISTER_SUCCESS);
    extract_msg(buffer,msg_len,MSG_REGISTER_SUCCESS,&result2);
    assert(result2 == result);
    free(buffer);
    buffer = 0;

    // register failure
    // printf("test register failure\n");
    assemble_msg(&buffer,&buffer_len,MSG_REGISTER_FAILURE,result);
    // print_buffer(buffer,buffer_len);
    extract_msg_len_type(&msg_len,&msg_type,buffer);
    assert(msg_len == buffer_len - 5);
    assert(msg_type == MSG_REGISTER_FAILURE);
    extract_msg(buffer,msg_len,MSG_REGISTER_FAILURE,&result2);
    assert(result2 == result);
    free(buffer);
    buffer = 0;

    // loc failure
    // printf("test loc failure\n");
    assemble_msg(&buffer,&buffer_len,MSG_LOC_FAILURE,result);
    // print_buffer(buffer,buffer_len);
    extract_msg_len_type(&msg_len,&msg_type,buffer);
    assert(msg_len == buffer_len - 5);
    assert(msg_type == MSG_LOC_FAILURE);
    extract_msg(buffer,msg_len,MSG_LOC_FAILURE,&result2);
    assert(result2 == result);
    free(buffer);
    buffer = 0;

    // execute failure
    // printf("test execute failure\n");
    assemble_msg(&buffer,&buffer_len,MSG_EXECUTE_FAILURE,result);
    // print_buffer(buffer,buffer_len);
    extract_msg_len_type(&msg_len,&msg_type,buffer);
    assert(msg_len == buffer_len - 5);
    assert(msg_type == MSG_EXECUTE_FAILURE);
    extract_msg(buffer,msg_len,MSG_EXECUTE_FAILURE,&result2);
    assert(result2 == result);
    free(buffer);
    buffer = 0;

}


void test_register() {

    // test assemble message
    char* buffer = 0;
    unsigned int buffer_len;
    int* arg_types = (int*) malloc(2*sizeof(int));
    int type1 = 0x1234;
    int type2 = 0x5678;
    arg_types[0] = type1;
    arg_types[1] = type2;

    unsigned int ip = 0xaaaa;
    unsigned int port = 0x00bb;

    unsigned int fct_name_len = 12;
    char fct_name[] = "abcd12341234";
    assemble_msg(&buffer,&buffer_len,
        MSG_REGISTER,ip,port,fct_name_len,fct_name,2,arg_types);

    assert(buffer_len == (4 + 1 + 4 + 2 + 4 + fct_name_len + 4 + 8 ) );

    // printf("buffer_len=%d\n",buffer_len);
    // print_buffer(buffer,buffer_len);

    char msg_type;
    unsigned int msg_len;
    extract_msg_len_type(&msg_len,&msg_type,buffer);
    assert(msg_len == buffer_len - 5);
    assert(msg_type == MSG_REGISTER);
    // printf("msg_len=%d\n",msg_len);

    unsigned int ip2, port2, fct_name2_len, num_args;
    char* fct_name2 = 0;
    int* arg_types2 = 0;

    extract_msg(buffer,buffer_len,
        MSG_REGISTER,&ip2,&port2,&fct_name2_len,&fct_name2,&num_args,&arg_types2);

    assert(ip2 == ip);
    assert(port2 == port);
    assert(fct_name2_len == fct_name_len);
    assert(mystrcmp(fct_name2,fct_name,fct_name_len) == 0);
    assert(arg_types2 != NULL);
    assert(arg_types2[0] == type1);
    assert(arg_types2[1] == type2);

    free(buffer);
    free(arg_types);
    free(fct_name2);
    free(arg_types2);
}


void test_loc_request() {

    // set up test
    char* buffer = 0;
    unsigned int buffer_len;

    unsigned int fct_name_len = 12;
    char fct_name[] = "abcd12341234";

    unsigned int arg_types_len = 2;
    int* arg_types = (int*) malloc(2*sizeof(int));
    arg_types[0] = 0x1234;
    arg_types[1] = 0x5678;

    unsigned int ip = 0xaaaa;
    unsigned int port = 0x00bb;

    // things to be extracted
    unsigned int msg_len2;
    char msg_type2;
    unsigned int fct_name2_len;
    char* fct_name2 = NULL;
    unsigned int arg_types2_len;
    int* arg_types2 = NULL;
    unsigned int ip2, port2;

    // test loc request
    assemble_msg(&buffer,&buffer_len,MSG_LOC_REQUEST,
        fct_name_len,fct_name,arg_types_len,arg_types);

    assert(buffer_len == (4 + 1 + 4 + fct_name_len + 4 + 8 ) );

    extract_msg_len_type(&msg_len2,&msg_type2,buffer);
    assert(msg_len2 == buffer_len - 5);
    assert(msg_type2 == MSG_LOC_REQUEST);

    extract_msg(buffer,buffer_len,MSG_LOC_REQUEST,
        &fct_name2_len,&fct_name2,&arg_types2_len,&arg_types2);

    assert(fct_name2_len == fct_name_len);
    assert(mystrcmp(fct_name2,fct_name,fct_name_len) == 0);
    assert(arg_types2 != NULL);
    assert(arg_types2[0] == arg_types[0]);
    assert(arg_types2[1] == arg_types[1]);

    free(fct_name2);
    free(arg_types2);
    free(buffer);

    // test loc request success
    buffer = NULL;
    assemble_msg(&buffer,&buffer_len,MSG_LOC_SUCCESS,
        ip,port);

    assert(buffer_len == (4 + 1 + 4 + 2 ) );

    extract_msg_len_type(&msg_len2,&msg_type2,buffer);
    assert(msg_len2 == buffer_len - 5);
    assert(msg_type2 == MSG_LOC_SUCCESS);

    extract_msg(buffer,buffer_len,MSG_LOC_SUCCESS,
        &ip2,&port2);

    assert(ip2 == ip);
    assert(port2 == port);

    free(buffer);
    // free(fct_name);
    free(arg_types);

}






int main() {
    //            AAAAAAAABBBBBBBBCCCCCCCCDDDDDDDD
    int type1 = 0b11000000000000110000000000000111;

    assertTypeEquals(type1,true,true,3,7);

    //                    AAAAAAAABBBBBBBBCCCCCCCCDDDDDDDD
    int type_invalid1 = 0b00000000000000110000000000000111; // not input nor output
    int type_invalid2 = 0b10000000000000000000000000000111; // invalid type
    int type_invalid3 = 0b10010110000000110000000000000111; // not padded by zeros

    assert(type_is_valid(type_invalid1) == false);
    assert(type_is_valid(type_invalid2) == false);
    assert(type_is_valid(type_invalid3) == false);

    test_short_messages();

    test_register();

    test_loc_request();


    return 0;
}

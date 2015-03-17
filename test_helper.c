#include "rpc.h"
#include "helper.h"
#include "defines.h"
#include "debug.h"

#include <stdlib.h>

#include <assert.h>
#include <string.h>

void assertTypeEquals(int type, bool is_in, bool is_out, char arg_type, unsigned int arg_size)
{

    bool temp_is_in;
    bool temp_is_out;
    char temp_arg_type;
    unsigned int temp_arg_size;

    assert( type_is_input(&temp_is_in,type) >= 0 );
    assert( type_is_output(&temp_is_out,type) >= 0 );
    assert( type_arg_type(&temp_arg_type,type) >= 0 );
    assert( type_arg_size(&temp_arg_size,type) >= 0 );

    assert( temp_is_in == is_in );
    assert( temp_is_out == is_out );
    assert( temp_arg_type == arg_type );
    assert( temp_arg_size == arg_size );
}

int mystrcmp(const char* const a, const char* const b, const unsigned int length)   // return 0 if same, or 1 if not same
{
    for ( unsigned int i = 0 ; i < length ; i += 1 ) {
        if ( a[i] != b[i] ) {
            return 1;
        }
    }
    return 0;
}

void test_short_messages()
{

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

    // loc cache failure
    // printf("test execute failure\n");
    assemble_msg(&buffer,&buffer_len,MSG_LOC_CACHE_FAILURE,result);
    // print_buffer(buffer,buffer_len);
    extract_msg_len_type(&msg_len,&msg_type,buffer);
    assert(msg_len == buffer_len - 5);
    assert(msg_type == MSG_EXECUTE_FAILURE);
    extract_msg(buffer,msg_len,MSG_EXECUTE_FAILURE,&result2);
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


void test_register()
{
    // printf("test register\n");

    // test assemble message
    char* buffer = 0;
    unsigned int buffer_len;
    int* arg_types = (int*) malloc(3*sizeof(int));
    int type1 = 0x1234;
    int type2 = 0x5678;
    arg_types[0] = type1;
    arg_types[1] = type2;
    arg_types[2] = 0;


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
    assert(arg_types2[2] == 0);

    free(buffer);
    free(arg_types);
    free(fct_name2);
    free(arg_types2);
}


void test_loc_request()
{
    // printf("test loc request\n");

    // set up test
    char* buffer = 0;
    unsigned int buffer_len;

    unsigned int fct_name_len = 12;
    char fct_name[] = "abcd12341234";

    unsigned int arg_types_len = 2;
    int* arg_types = (int*) malloc(3*sizeof(int));
    arg_types[0] = 0x1234;
    arg_types[1] = 0x5678;
    arg_types[2] = 0;

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
    assert(arg_types2[2] == 0);

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

void test_execute_with_execute_type(char type) {

    // set up test
    char* buffer = 0;
    unsigned int buffer_len;

    unsigned int fct_name_len = 12;
    char fct_name[] = "abcd12341234";

    unsigned int arg_types_len = 4;
    int* arg_types = (int*) malloc(5*sizeof(int));
    //               AAAAAAAABBBBBBBBCCCCCCCCDDDDDDDD
    arg_types[0] = 0b11000000000000110000000000000010; // array of 2 ints
    arg_types[1] = 0b11000000000001010000000000000000; // double
    arg_types[2] = 0b11000000000001010000000000000010; // array of 2 doubles
    arg_types[3] = 0b11000000000000010000000000000000; // char
    arg_types[4] = 0;

    void** args = (void**)malloc(4*sizeof(void*));

    int int_a[2] = {0x10,0x20};
    double double_a = 5000.02;
    double double_b[2] = { 5000.02 , 5000.02 };
    char char_a = 'z';

    args[0] = (void*)int_a;
    args[1] = (void*)&double_a;
    args[2] = (void*)double_b;
    args[3] = (void*)&char_a;

    // things to be extracted
    unsigned int msg_len2;
    char msg_type2;
    unsigned int fct_name2_len;
    char* fct_name2 = NULL;
    unsigned int arg_types2_len;
    int* arg_types2 = NULL;
    void** args2 = NULL;

    // things that get checked with the original values
    int *int_a2 = (int*)malloc(2*sizeof(int));
    double double_a2;
    double* double_b2 = (double*)malloc(2*sizeof(double));
    char char_a2 = 'z';

    void** args3 = (void**)malloc(4*sizeof(void*));
    args3[0] = (void*)int_a2;
    args3[1] = (void*)&double_a2;
    args3[2] = (void*)double_b2;
    args3[3] = (void*)&char_a2;

    // assemble
    assemble_msg(&buffer,&buffer_len,type,
                 fct_name_len,fct_name,arg_types_len,arg_types,args);

    // check buffer_len
    // printf("buffer_len:%d\n",buffer_len);
    assert(buffer_len == (4 + 1 + 4 + fct_name_len + 4 + 16 + 33 ) );

    // extract len + type
    // printf("execute extract len type\n");
    extract_msg_len_type(&msg_len2,&msg_type2,buffer);
    assert(msg_len2 == buffer_len - 5);
    assert(msg_type2 == type);
    // print_buffer(buffer,buffer_len);

    // extract
    // printf("execute extract\n");
    extract_msg(buffer,buffer_len,type,
                &fct_name2_len,&fct_name2,&arg_types2_len,&arg_types2,&args2);
    assert(fct_name2_len == fct_name_len);
    assert(mystrcmp(fct_name2,fct_name,fct_name_len) == 0);
    assert(arg_types2 != NULL);
    assert(arg_types2[0] == arg_types[0]);
    assert(arg_types2[1] == arg_types[1]);
    assert(arg_types2[2] == arg_types[2]);
    assert(arg_types2[3] == arg_types[3]);
    assert(arg_types2[4] == 0);
    assert(args != NULL);
    assert(args2[0] != NULL);
    assert(args2[1] != NULL);
    assert(args2[2] != NULL);
    assert(args2[3] != NULL);

    // printf("%x =? %x\n",((int*)args2[0])[0],int_a[0]);

    // printf("%x =? %x\n",((int*)args2[0])[0],((int*)args[0])[0]);
    // printf("%x =? %x\n",((int*)args2[0])[1],((int*)args[0])[1]);
    // printf("%f =? %f\n",((double*)args2[1])[0],((double*)args[1])[0]);
    // printf("%f =? %f\n",((double*)args2[2])[0],((double*)args[2])[0]);
    // printf("%f =? %f\n",((double*)args2[2])[1],((double*)args[2])[1]);
    // printf("%c =? %c\n",((char*)args2[3])[0],((char*)args[3])[0]);

    // THIS IS A BAD CHECK {
    assert( ((int*)args2[0]) != ((int*)args[0]) ); // make sure it's not the same array
    assert( ((int*)args2[0])[0] == ((int*)args[0])[0] );
    assert( ((int*)args2[0])[1] == ((int*)args[0])[1] );
    assert( ((double*)args2[1])[0] == ((double*)args[1])[0] );
    assert( ((double*)args2[2])[0] == ((double*)args[2])[0] );
    assert( ((double*)args2[2])[1] == ((double*)args[2])[1] );
    assert( ((char*)args2[3])[0] == ((char*)args[3])[0] );
    // }

    copy_args_step_by_step(arg_types2,args3,args2);

    assert(int_a2[0] = int_a[0]);
    assert(int_a2[1] = int_a[1]);
    assert(double_a2 = double_a);
    assert(double_b2[0] = double_b[0]);
    assert(double_b2[1] = double_b[1]);
    assert(char_a2 = char_a);

    // cleanup
    free(buffer);
    free(fct_name2);
    free(arg_types2);
    free(args2[0]);
    free(args2[1]);
    free(args2[2]);
    free(args2[3]);
    free(args2);

    free(int_a2);
    free(double_b2);
    free(args3);

    free(arg_types);
    free(args);

}



void test_execute()
{

    // printf("test execute\n");
    test_execute_with_execute_type(MSG_EXECUTE);

    test_execute_with_execute_type(MSG_EXECUTE_SUCCESS);

}

void test_loc_cache_request() {

    char *return2 = (char *)malloc(25 * sizeof(char));
    unsigned int fct_name_len = 5;
    char fct_name[] = "fct01";
    unsigned int arg_types_len =3;
    int arg_types[arg_types_len + 1];

    arg_types[0] = (1 << ARG_OUTPUT) | (ARG_CHAR << 16) | 25;
    arg_types[1] = (1 << ARG_INPUT) | (ARG_FLOAT << 16);
    arg_types[2] = (1 << ARG_INPUT) | (ARG_DOUBLE << 16);
    arg_types[3] = 0;

    // buffer stuff
    char* buffer = NULL;
    unsigned int buffer_len;

    // things to be extracted
    unsigned int msg_len2;
    char msg_type2;
    unsigned int fct_name2_len;
    char* fct_name2 = NULL;
    unsigned int arg_types2_len;
    int* arg_types2 = NULL;

    // assemble
    assemble_msg(&buffer,&buffer_len,MSG_LOC_CACHE_REQUEST,
                 fct_name_len,fct_name,arg_types_len,arg_types);

    assert(buffer_len == (4 + 1 + 4 + fct_name_len + 4 + 25 + 4 + 8 ) );

    // extract len + type
    // printf("execute extract len type\n");
    extract_msg_len_type(&msg_len2,&msg_type2,buffer);
    assert(msg_len2 == buffer_len - 5);
    assert(msg_type2 == MSG_LOC_CACHE_REQUEST);

    extract_msg(buffer,buffer_len,MSG_LOC_CACHE_REQUEST,
                &fct_name2_len,&fct_name2,&arg_types2_len,&arg_types2);

    assert(fct_name2_len == fct_name_len);
    assert(mystrcmp(fct_name2,fct_name,fct_name_len) == 0);
    assert(arg_types2_len == arg_types_len);
    assert(arg_types2 != NULL);
    assert(arg_types2[0] == arg_types[0]);
    assert(arg_types2[1] == arg_types[1]);
    assert(arg_types2[2] == arg_types[2]);
    assert(arg_types2[3] == 0);

    free(buffer);
    free(arg_types2);
    free(fct_name2);

    free(return2);

}

void test_loc_cache_success() {

    char* buffer = NULL;
    unsigned int buffer_len;

    unsigned int hosts_len = 3;
    int ips[] = { 0xAAAAAAAA , 0xBBBBBBBB , 0xCCCCCCCC };
    int ports[] = { 0x1111 , 0x2222 , 0x3333 };

    // assemble
    assemble_msg(&buffer,&buffer_len,MSG_LOC_CACHE_SUCCESS,
        hosts_len, ips, ports);

    assert(buffer_len == (4 + 1 + 4 + 6*hosts_len ) );

    // things to be extracted
    unsigned int msg_len2;
    char msg_type2;
    unsigned int hosts_len2;
    int* ips2 = NULL;
    int* ports2 = NULL;

    extract_msg_len_type(&msg_len2,&msg_type2,buffer);
    assert(msg_len2 == buffer_len - 5);
    assert(msg_type2 == MSG_LOC_CACHE_SUCCESS);

    extract_msg(buffer,buffer_len,MSG_LOC_CACHE_SUCCESS,
        &hosts_len2,&ips2,&ports2);

    assert(hosts_len2 == hosts_len);
    assert(ips2 != NULL);
    assert(ports2 != NULL);

    for ( unsigned int i = 0 ; i < hosts_len2 ; i += 1 ) {
        assert(ips2[i] == ips[i]);
        assert(ports2[i] == ports[i]);
    }

    free(ips2);
    free(ports2);

    free(buffer);

}

int test_copy_args() {

    int char_array_len = 100;

    // setup the args (taken from given client)
    float a_float = 3.14159;
    double a_double = 1234.1001;
    int args_count = 3;
    char *char_array = (char *)malloc((char_array_len) * sizeof(char));
    memset(char_array,0,char_array_len);
    memcpy(char_array,"abcdefg",7);
    int argTypes[args_count + 1];
    void **args;

    argTypes[0] = (1 << ARG_OUTPUT) | (ARG_CHAR << 16) | char_array_len;
    argTypes[1] = (1 << ARG_INPUT) | (ARG_FLOAT << 16);
    argTypes[2] = (1 << ARG_INPUT) | (ARG_DOUBLE << 16);
    argTypes[3] = 0;

    args = (void **)malloc(args_count * sizeof(void *));
    args[0] = (void *)char_array;
    args[1] = (void *)&a_float;
    args[2] = (void *)&a_double;

    // allocate new buffer
    char *new_char_array = (char *)malloc(char_array_len * sizeof(char));
    memset(new_char_array,0,char_array_len);
    float new_a_float = 0;
    double new_a_double = 0;
    void** new_args;

    new_args = (void **)malloc(args_count * sizeof(void *));
    new_args[0] = (void *)new_char_array;
    new_args[1] = (void *)&new_a_float;
    new_args[2] = (void *)&new_a_double;

    // printf("pointer address?\n");
    // printf("new_char_array = %p\n",(void*)&new_char_array);
    // printf("new_a_float    = %p\n",(void*)&new_a_float);
    // printf("new_a_double   = %p\n",(void*)&new_a_double);
    // printf("1              = %p\n",(void*)new_args[0]);
    // printf("2              = %p\n",(void*)new_args[1]);
    // printf("3              = %p\n",(void*)new_args[2]);

    // printf("expected\n");
    // print_buffer((char*)args[0],100);
    // print_buffer((char*)args[1],16);
    // print_buffer((char*)args[2],16);

    // copy
    assert(copy_args_step_by_step(argTypes,new_args,args) == 0);

    // printf("actual\n");
    // print_buffer((char*)new_args[0],100);
    // print_buffer((char*)new_args[1],16);
    // print_buffer((char*)new_args[2],16);

    // new_args[1] = (void*)malloc(sizeof(float));
    // *(float*)new_args[1] = 123;
    // printf("%f == %f \n", new_a_float, a_float);

    // check if they are the same
    assert( mystrcmp(new_char_array,char_array,char_array_len) == 0 );
    assert( new_a_float == a_float );
    assert( new_a_double == a_double );

    // cleanup
    free(char_array);
    free(args);
    free(new_char_array);
    free(new_args);

    return 0;
}



int main()
{

    //            AAAAAAAABBBBBBBBCCCCCCCCDDDDDDDD
    int type1 = 0b11000000000000110000000000000111;
    int type2 = 0b11000000000001010000000000000000;

    assertTypeEquals(type1,true,true,3,7);
    assert(type_arg_total_length(type1) == 28);
    assert(type_is_array(type1) == true);

    assertTypeEquals(type2,true,true,5,0);
    assert(type_arg_total_length(type2) == 8);
    assert(type_is_array(type2) == false);

    //                    AAAAAAAABBBBBBBBCCCCCCCCDDDDDDDD
    int type_invalid1 = 0b00000000000000110000000000000111; // not input nor output
    int type_invalid2 = 0b10000000000000000000000000000111; // invalid type
    int type_invalid3 = 0b10010110000000110000000000000111; // not padded by zeros

    assert(type_is_valid(type_invalid1) == false);
    assert(type_is_valid(type_invalid2) == false);
    assert(type_is_valid(type_invalid3) == false);
    assert(type_arg_total_length(type_invalid2) == 0);

    for ( int i = 0 ; i < 5 ; i += 1 ) {
        test_short_messages();
        test_register();
        test_loc_request();
        test_loc_cache_request();
        test_loc_cache_success();
        test_execute();
    }

    test_copy_args();

    return 0;
}

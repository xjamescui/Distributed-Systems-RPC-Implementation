#include "rpc.h"
#include "helper.h"

#include <assert.h>

void assertTypeEquals(int type, bool is_in, bool is_out, char arg_type, int arg_size) {

    bool temp_is_in;
    bool temp_is_out;
    char temp_arg_type;
    int temp_arg_size;

    assert( type_is_input(temp_is_in,type) >= 0 );
    assert( type_is_output(temp_is_out,type) >= 0 );
    assert( type_arg_type(temp_arg_type,type) >= 0 );
    assert( type_arg_size(temp_arg_size,type) >= 0 );

    assert( temp_is_in == is_in );
    assert( temp_is_out == is_out );
    assert( temp_arg_type == arg_type );
    assert( temp_arg_size == arg_size );
}





int main() {
    //            AAAAAAAABBBBBBBBCCCCCCCCDDDDDDDD
    int type1 = 0b11000000000000110000000000000111;

    assertTypeEquals(type1,true,true,3,7);

    return 0;
}

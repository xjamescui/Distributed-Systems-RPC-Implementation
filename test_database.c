#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "database.h"


int main() {

    unsigned int ip;
    unsigned int port;
    SIGNATURE sig;
    sig.fct_name_len = 5;
    sig.fct_name = (char*)malloc(32);
    memcpy(sig.fct_name,"fct01",5);

    sig.arg_types_len = 2;
    sig.arg_types = (int*)malloc(32*4);
    sig.arg_types[0] = 1;
    sig.arg_types[1] = 2;

    unsigned int size;

    // get empty
    printf("test get empty\n");
    db_size(&size);
    assert(size == 0);
    assert(db_get(&ip,&port,sig) == SIGNATURE_NOT_FOUND );
    assert(db_size(&size) == 0 && size == 0);

    ip = 111;
    port = 222;

    // insert it and try again
    unsigned int ip2 = 111;
    unsigned int port2 = 222;

    SIGNATURE sig2;
    sig2.fct_name_len = 5;
    sig2.fct_name = (char*)malloc(32);
    memcpy(sig2.fct_name,"fct02",5);

    sig2.arg_types_len = 2;
    sig2.arg_types = (int*)malloc(32*4);
    sig2.arg_types[0] = 1;
    sig2.arg_types[1] = 2;

    printf("test put duplicate\n");
    assert(db_put(ip2,port2,sig2) == SIGNATURE_PUT_SUCCESS);
    assert(db_put(ip2,port2,sig2) == SIGNATURE_PUT_DUPLICATE);
    assert(db_put(ip2,port2,sig2) == SIGNATURE_PUT_DUPLICATE);
    assert(db_put(ip2,port2,sig2) == SIGNATURE_PUT_DUPLICATE);
    assert(db_put(ip2,port2,sig2) == SIGNATURE_PUT_DUPLICATE);
    assert(db_size(&size) == 0 && size == 1);


    // try 2 hosts
    unsigned int ip3 = 112;
    unsigned int port3 = 223;

    printf("test put non-duplicate\n");
    assert(db_put(ip3,port3,sig2) == SIGNATURE_PUT_SUCCESS);
    assert(db_size(&size) ==0 && size == 2);

    assert(db_put(ip3,port3,sig2) == SIGNATURE_PUT_DUPLICATE);
    assert(db_put(ip3,port3,sig2) == SIGNATURE_PUT_DUPLICATE);
    assert(db_put(ip3,port3,sig2) == SIGNATURE_PUT_DUPLICATE);
    assert(db_size(&size) ==0 && size == 2);

    // get
    unsigned int get_ip;
    unsigned int get_port;

    printf("test get not found\n");
    assert(db_get(&get_ip,&get_port,sig) == SIGNATURE_NOT_FOUND);
    assert(db_get(&get_ip,&get_port,sig) == SIGNATURE_NOT_FOUND);
    assert(db_get(&get_ip,&get_port,sig) == SIGNATURE_NOT_FOUND);

    printf("test get found\n");
    assert(db_get(&get_ip,&get_port,sig2) == SIGNATURE_FOUND);
    assert(get_ip == ip2 && get_port == port2 );
    assert(db_get(&get_ip,&get_port,sig2) == SIGNATURE_FOUND);
    assert(get_ip == ip3 && get_port == port3 );
    assert(db_get(&get_ip,&get_port,sig2) == SIGNATURE_FOUND);
    assert(get_ip == ip2 && get_port == port2 );
    assert(db_get(&get_ip,&get_port,sig2) == SIGNATURE_FOUND);
    assert(get_ip == ip3 && get_port == port3 );

    // delete then get
    printf("test get found , host not found\n");
    assert(db_delete_host(ip2,port2,sig) == SIGNATURE_NOT_FOUND);
    assert(db_delete_host(ip2,port2,sig2) == DELETE_HOST_SUCCESS);
    assert(db_delete_host(ip2,port2,sig2) == DELETE_HOST_NOT_FOUND);
    assert(db_delete_host(ip2,port3,sig2) == DELETE_HOST_NOT_FOUND);
    assert(db_delete_host(ip3,port3,sig2) == DELETE_HOST_SUCCESS);
    assert(db_get(&get_ip,&get_port,sig2) == SIGNATURE_HAS_NO_HOSTS);

    // test drop
    printf("test drop\n");
    assert(db_drop() == 0);
    assert(db_size(&size) ==0 && size == 0);

    free(sig.fct_name);
    free(sig.arg_types);
    free(sig2.fct_name);
    free(sig2.arg_types);

    printf("test given\n");
    // try the given example
    // A:f,g,h
    // B:f,g
    // req: fhgf -> AABA
    SIGNATURE sig_f;
    sig_f.fct_name_len = 10;
    sig_f.fct_name = (char*)malloc(sig_f.fct_name_len);
    memcpy(sig_f.fct_name,"fct_f",sig_f.fct_name_len);
    sig_f.arg_types_len = 2;
    sig_f.arg_types = (int*)malloc(sig_f.arg_types_len*4);
    sig_f.arg_types[0] = 1; 
    sig_f.arg_types[1] = 2;

    SIGNATURE sig_g;
    sig_g.fct_name_len = 10;
    sig_g.fct_name = (char*)malloc(sig_g.fct_name_len);
    memcpy(sig_g.fct_name,"fct_g",sig_g.fct_name_len);
    sig_g.arg_types_len = 1;
    sig_g.arg_types = (int*)malloc(sig_g.fct_name_len*4);
    sig_g.arg_types[0] = 20;

    SIGNATURE sig_h;
    sig_h.fct_name_len = 10;
    sig_h.fct_name = (char*)malloc(sig_h.fct_name_len);
    memcpy(sig_h.fct_name,"fct_h",sig_h.fct_name_len);
    sig_h.arg_types_len = 1;
    sig_h.arg_types = (int*)malloc(sig_h.fct_name_len*4);
    sig_h.arg_types[0] = 21;

    unsigned int ip_a = 0xaa11;
    unsigned int port_a = 0xaa22;
    unsigned int ip_b = 0xbb11;
    unsigned int port_b = 0xbb22;

    assert(db_put(ip_a,port_a,sig_f) == SIGNATURE_PUT_SUCCESS);
    assert(db_put(ip_a,port_a,sig_g) == SIGNATURE_PUT_SUCCESS);
    assert(db_put(ip_a,port_a,sig_h) == SIGNATURE_PUT_SUCCESS);
    assert(db_put(ip_b,port_b,sig_f) == SIGNATURE_PUT_SUCCESS);
    assert(db_put(ip_b,port_b,sig_g) == SIGNATURE_PUT_SUCCESS);
    assert(db_size(&size) ==0 && size == 5);

    // db_print();

    assert(db_get(&get_ip,&get_port,sig_f) == SIGNATURE_FOUND);
    assert(get_ip == ip_a && get_port == port_a );
    // db_print();
    assert(db_get(&get_ip,&get_port,sig_h) == SIGNATURE_FOUND);
    assert(get_ip == ip_a && get_port == port_a );
    // db_print();
    assert(db_get(&get_ip,&get_port,sig_g) == SIGNATURE_FOUND);
    assert(get_ip == ip_b && get_port == port_b );
    // db_print();
    assert(db_get(&get_ip,&get_port,sig_f) == SIGNATURE_FOUND);
    assert(get_ip == ip_a && get_port == port_a );
    // db_print();

    assert(db_drop() == 0);
    assert(db_size(&size) ==0 && size == 0);

    free(sig_f.fct_name);
    free(sig_f.arg_types);
    free(sig_g.fct_name);
    free(sig_g.arg_types);
    free(sig_h.fct_name);
    free(sig_h.arg_types);



}
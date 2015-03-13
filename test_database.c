#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "binder_database.h"


int main() {

    HOST host_1;

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
    assert(db_get(&host_1,sig) == BINDER_DB_GET_SIGNATURE_NOT_FOUND);
    assert(db_size(&size) == 0 && size == 0);

    // insert it and try again
    HOST host_2;
    host_2.sock_fd = 2;
    host_2.ip = 0x2222;
    host_2.port = 0x22;

    SIGNATURE sig2;
    sig2.fct_name_len = 5;
    sig2.fct_name = (char*)malloc(32);
    memcpy(sig2.fct_name,"fct02",5);

    sig2.arg_types_len = 2;
    sig2.arg_types = (int*)malloc(32*4);
    sig2.arg_types[0] = 1;
    sig2.arg_types[1] = 2;

    printf("test put duplicate\n");
    assert(db_put(host_2,sig2) == BINDER_DB_PUT_SIGNATURE_SUCCESS);
    assert(db_put(host_2,sig2) == BINDER_DB_PUT_SIGNATURE_DUPLICATE);
    assert(db_put(host_2,sig2) == BINDER_DB_PUT_SIGNATURE_DUPLICATE);
    assert(db_put(host_2,sig2) == BINDER_DB_PUT_SIGNATURE_DUPLICATE);
    assert(db_put(host_2,sig2) == BINDER_DB_PUT_SIGNATURE_DUPLICATE);
    assert(db_size(&size) == 0 && size == 1);


    // try 2 hosts
    HOST host_3;
    host_3.sock_fd = 3;
    host_3.ip = 0x3333;
    host_3.port = 0x33;

    printf("test put non-duplicate\n");
    assert(db_put(host_3,sig2) == BINDER_DB_PUT_SIGNATURE_SUCCESS);
    assert(db_size(&size) ==0 && size == 2);

    assert(db_put(host_3,sig2) == BINDER_DB_PUT_SIGNATURE_DUPLICATE);
    assert(db_put(host_3,sig2) == BINDER_DB_PUT_SIGNATURE_DUPLICATE);
    assert(db_put(host_3,sig2) == BINDER_DB_PUT_SIGNATURE_DUPLICATE);
    assert(db_size(&size) ==0 && size == 2);

    // get
    printf("test get not found\n");
    assert(db_get(&host_1,sig) == BINDER_DB_GET_SIGNATURE_NOT_FOUND);
    assert(db_get(&host_1,sig) == BINDER_DB_GET_SIGNATURE_NOT_FOUND);
    assert(db_get(&host_1,sig) == BINDER_DB_GET_SIGNATURE_NOT_FOUND);

    printf("test get found\n");
    assert(db_get(&host_1,sig2) == BINDER_DB_GET_SIGNATURE_FOUND);
    assert(host_1.sock_fd == host_2.sock_fd && host_1.ip == host_2.ip && host_1.port == host_2.port );
    assert(db_get(&host_1,sig2) == BINDER_DB_GET_SIGNATURE_FOUND);
    assert(host_1.sock_fd == host_3.sock_fd && host_1.ip == host_3.ip && host_1.port == host_3.port );
    assert(db_get(&host_1,sig2) == BINDER_DB_GET_SIGNATURE_FOUND);
    assert(host_1.sock_fd == host_2.sock_fd && host_1.ip == host_2.ip && host_1.port == host_2.port );
    assert(db_get(&host_1,sig2) == BINDER_DB_GET_SIGNATURE_FOUND);
    assert(host_1.sock_fd == host_3.sock_fd && host_1.ip == host_3.ip && host_1.port == host_3.port );

    // delete then get
    printf("test get found , host not found\n");
    assert(db_delete_host(host_2,sig) == BINDER_DB_DELETE_SIGNATURE_NOT_FOUND);
    assert(db_delete_host(host_2,sig2) == BINDER_DB_DELETE_HOST_SUCCESS);
    assert(db_delete_host(host_2,sig2) == BINDER_DB_DELETE_HOST_NOT_FOUND);
    assert(db_delete_host(host_3,sig2) == BINDER_DB_DELETE_HOST_SUCCESS);
    assert(db_get(&host_1,sig2) == BINDER_DB_GET_SIGNATURE_HAS_NO_HOSTS);

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

    HOST host_a;
    host_a.sock_fd = 10;
    host_a.ip = 0xaa11;
    host_a.port = 0xaa22;

    HOST host_b;
    host_b.sock_fd = 11;
    host_b.ip = 0xbb11;
    host_b.port = 0xbb22;

    assert(db_put(host_a,sig_f) == BINDER_DB_PUT_SIGNATURE_SUCCESS);
    assert(db_put(host_a,sig_g) == BINDER_DB_PUT_SIGNATURE_SUCCESS);
    assert(db_put(host_a,sig_h) == BINDER_DB_PUT_SIGNATURE_SUCCESS);
    assert(db_put(host_b,sig_f) == BINDER_DB_PUT_SIGNATURE_SUCCESS);
    assert(db_put(host_b,sig_g) == BINDER_DB_PUT_SIGNATURE_SUCCESS);
    assert(db_size(&size) ==0 && size == 5);

    // db_print();

    assert(db_get(&host_1,sig_f) == BINDER_DB_GET_SIGNATURE_FOUND);
    assert(host_1.sock_fd == host_a.sock_fd && host_1.ip == host_a.ip && host_1.port == host_a.port );
    // db_print();
    assert(db_get(&host_1,sig_h) == BINDER_DB_GET_SIGNATURE_FOUND);
    assert(host_1.sock_fd == host_a.sock_fd && host_1.ip == host_a.ip && host_1.port == host_a.port );
    // db_print();
    assert(db_get(&host_1,sig_g) == BINDER_DB_GET_SIGNATURE_FOUND);
    assert(host_1.sock_fd == host_b.sock_fd && host_1.ip == host_b.ip && host_1.port == host_b.port );
    // db_print();
    assert(db_get(&host_1,sig_f) == BINDER_DB_GET_SIGNATURE_FOUND);
    assert(host_1.sock_fd == host_a.sock_fd && host_1.ip == host_a.ip && host_1.port == host_a.port );
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

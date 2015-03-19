#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "host_database.h"


unsigned int intersect(unsigned int const *const array1, unsigned int const array1_len, unsigned int const *const array2, unsigned int const array2_len)
{
    unsigned int count = 0;
    for( unsigned int  i = 0 ; i < array1_len ; i += 1 ) {
        for(unsigned int  j = 0; j < array2_len ; j += 1 ) {
            if( array1[i] == array2[j] ) {
                count += 1;
            }
        }
    }
    return count;
}

void test_normal() {

    HOST host_1;

    SIGNATURE sig;
    sig.fct_name_len = 5;
    sig.fct_name = (char*)malloc(32);
    memcpy(sig.fct_name,"fct01",5);

    sig.arg_types_len = 2;
    sig.arg_types = (int*)malloc(32*4);
    sig.arg_types[0] = 1;
    sig.arg_types[1] = 2;

    // get empty
    // printf("test get empty\n");
    assert(db_get(&host_1,sig) == HOST_DB_GET_SIGNATURE_NOT_FOUND);
    assert(db_size() == 0);

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

    // printf("test put duplicate\n");
    assert(db_put(host_2,sig2) == HOST_DB_PUT_SIGNATURE_SUCCESS);
    assert(db_put(host_2,sig2) == HOST_DB_PUT_SIGNATURE_DUPLICATE);
    assert(db_put(host_2,sig2) == HOST_DB_PUT_SIGNATURE_DUPLICATE);
    assert(db_put(host_2,sig2) == HOST_DB_PUT_SIGNATURE_DUPLICATE);
    assert(db_put(host_2,sig2) == HOST_DB_PUT_SIGNATURE_DUPLICATE);
    assert(db_size() == 1);


    // try 2 hosts
    HOST host_3;
    host_3.sock_fd = 3;
    host_3.ip = 0x3333;
    host_3.port = 0x33;

    // printf("test put non-duplicate\n");
    assert(db_put(host_3,sig2) == HOST_DB_PUT_SIGNATURE_SUCCESS);
    assert(db_size() == 2);

    assert(db_put(host_3,sig2) == HOST_DB_PUT_SIGNATURE_DUPLICATE);
    assert(db_put(host_3,sig2) == HOST_DB_PUT_SIGNATURE_DUPLICATE);
    assert(db_put(host_3,sig2) == HOST_DB_PUT_SIGNATURE_DUPLICATE);
    assert(db_size() == 2);

    // get
    // printf("test get not found\n");
    assert(db_get(&host_1,sig) == HOST_DB_GET_SIGNATURE_NOT_FOUND);
    assert(db_get(&host_1,sig) == HOST_DB_GET_SIGNATURE_NOT_FOUND);
    assert(db_get(&host_1,sig) == HOST_DB_GET_SIGNATURE_NOT_FOUND);

    // printf("test get found\n");
    assert(db_get(&host_1,sig2) == HOST_DB_GET_SIGNATURE_FOUND);
    assert(host_1.sock_fd == host_2.sock_fd && host_1.ip == host_2.ip && host_1.port == host_2.port );
    assert(db_get(&host_1,sig2) == HOST_DB_GET_SIGNATURE_FOUND);
    assert(host_1.sock_fd == host_3.sock_fd && host_1.ip == host_3.ip && host_1.port == host_3.port );
    assert(db_get(&host_1,sig2) == HOST_DB_GET_SIGNATURE_FOUND);
    assert(host_1.sock_fd == host_2.sock_fd && host_1.ip == host_2.ip && host_1.port == host_2.port );
    assert(db_get(&host_1,sig2) == HOST_DB_GET_SIGNATURE_FOUND);
    assert(host_1.sock_fd == host_3.sock_fd && host_1.ip == host_3.ip && host_1.port == host_3.port );

    // delete then get
    // printf("test get found , host not found\n");
    assert(db_delete_host(host_2,sig) == HOST_DB_DELETE_SIGNATURE_NOT_FOUND);
    assert(db_delete_host(host_2,sig2) == HOST_DB_DELETE_HOST_SUCCESS);
    assert(db_delete_host(host_2,sig2) == HOST_DB_DELETE_HOST_NOT_FOUND);
    assert(db_delete_host(host_3,sig2) == HOST_DB_DELETE_HOST_SUCCESS);
    assert(db_get(&host_1,sig2) == HOST_DB_GET_SIGNATURE_HAS_NO_HOSTS);

    // test drop
    // printf("test drop\n");
    assert(db_drop() == 0);
    assert(db_size() == 0);

    free(sig.fct_name);
    free(sig.arg_types);
    free(sig2.fct_name);
    free(sig2.arg_types);

    // printf("test given\n");
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

    assert(db_put(host_a,sig_f) == HOST_DB_PUT_SIGNATURE_SUCCESS);
    assert(db_put(host_a,sig_g) == HOST_DB_PUT_SIGNATURE_SUCCESS);
    assert(db_put(host_a,sig_h) == HOST_DB_PUT_SIGNATURE_SUCCESS);
    assert(db_put(host_b,sig_f) == HOST_DB_PUT_SIGNATURE_SUCCESS);
    assert(db_put(host_b,sig_g) == HOST_DB_PUT_SIGNATURE_SUCCESS);
    assert(db_size() == 5);

    // db_print();

    assert(db_get(&host_1,sig_f) == HOST_DB_GET_SIGNATURE_FOUND);
    assert(host_1.sock_fd == host_a.sock_fd && host_1.ip == host_a.ip && host_1.port == host_a.port );
    // db_print();
    assert(db_get(&host_1,sig_h) == HOST_DB_GET_SIGNATURE_FOUND);
    assert(host_1.sock_fd == host_a.sock_fd && host_1.ip == host_a.ip && host_1.port == host_a.port );
    // db_print();
    assert(db_get(&host_1,sig_g) == HOST_DB_GET_SIGNATURE_FOUND);
    assert(host_1.sock_fd == host_b.sock_fd && host_1.ip == host_b.ip && host_1.port == host_b.port );
    // db_print();
    assert(db_get(&host_1,sig_f) == HOST_DB_GET_SIGNATURE_FOUND);
    assert(host_1.sock_fd == host_a.sock_fd && host_1.ip == host_a.ip && host_1.port == host_a.port );
    // db_print();

    // test db_get_all
    unsigned int hosts_len;
    unsigned int *ips;
    unsigned int *ports;
    assert(db_get_all(&hosts_len,&ips,&ports,sig) == HOST_DB_GET_SIGNATURE_NOT_FOUND);
    assert(db_get_all(&hosts_len,&ips,&ports,sig_f) == HOST_DB_GET_SIGNATURE_FOUND);
    assert(hosts_len == 2);

    unsigned int orig_ips[] = { host_a.ip, host_b.ip };
    unsigned int orig_ports[] = { host_a.port, host_b.port };

    assert(intersect(ips,hosts_len,orig_ips,2) == hosts_len);
    assert(intersect(ports,hosts_len,orig_ports,2) == hosts_len);

    free(ips);
    free(ports);

    // test db_drop
    assert(db_drop() == 0);
    assert(db_size() == 0);

    free(sig_f.fct_name);
    free(sig_f.arg_types);
    free(sig_g.fct_name);
    free(sig_g.arg_types);
    free(sig_h.fct_name);
    free(sig_h.arg_types);


}

void test_delete_all_for_sock() {

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


    HOST host_a = { 10 , 0xaaaa, 0x1111};
    HOST host_b = { 11 , 0xbbbb, 0x2222};

    assert(db_put(host_a,sig_f) == HOST_DB_PUT_SIGNATURE_SUCCESS);
    assert(db_put(host_a,sig_g) == HOST_DB_PUT_SIGNATURE_SUCCESS);
    assert(db_put(host_b,sig_f) == HOST_DB_PUT_SIGNATURE_SUCCESS);
    assert(db_put(host_b,sig_g) == HOST_DB_PUT_SIGNATURE_SUCCESS);
    assert(db_size() == 4);

    assert(db_delete_all_for_sock(10) == 2);
    assert(db_size() == 2);

    assert(db_delete_all_for_sock(11) == 2);
    assert(db_size() == 0);

    db_drop();

    free(sig_f.fct_name);
    free(sig_f.arg_types);
    free(sig_g.fct_name);
    free(sig_g.arg_types);

}

void test_round_robin() {

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

    HOST host_a = { 10 , 0xaaaa, 0x1111};
    HOST host_b = { 11 , 0xbbbb, 0x2222};
    HOST host_c = { 12 , 0xcccc, 0x3333};

    // sig_f : a->b->c
    // sig_g : b->a->c
    // get(a)
    // expect:
    // sig_f : b->c->a
    // sig_g : b->c->a

    assert(db_put(host_a,sig_f) == HOST_DB_PUT_SIGNATURE_SUCCESS);
    assert(db_put(host_b,sig_f) == HOST_DB_PUT_SIGNATURE_SUCCESS);
    assert(db_put(host_c,sig_f) == HOST_DB_PUT_SIGNATURE_SUCCESS);

    assert(db_put(host_b,sig_g) == HOST_DB_PUT_SIGNATURE_SUCCESS);
    assert(db_put(host_a,sig_g) == HOST_DB_PUT_SIGNATURE_SUCCESS);
    assert(db_put(host_c,sig_g) == HOST_DB_PUT_SIGNATURE_SUCCESS);

    assert(db_size() == 6);

    HOST get_host;

    assert(db_get(&get_host,sig_f) == HOST_DB_GET_SIGNATURE_FOUND);
    assert(get_host.sock_fd == host_a.sock_fd && get_host.ip == host_a.ip && get_host.port == host_a.port );
    assert(db_get(&get_host,sig_g) == HOST_DB_GET_SIGNATURE_FOUND);
    assert(get_host.sock_fd == host_b.sock_fd && get_host.ip == host_b.ip && get_host.port == host_b.port );
    assert(db_get(&get_host,sig_g) == HOST_DB_GET_SIGNATURE_FOUND);
    assert(get_host.sock_fd == host_c.sock_fd && get_host.ip == host_c.ip && get_host.port == host_c.port );
    assert(db_get(&get_host,sig_g) == HOST_DB_GET_SIGNATURE_FOUND);
    assert(get_host.sock_fd == host_a.sock_fd && get_host.ip == host_a.ip && get_host.port == host_a.port );

    db_drop();

    free(sig_f.fct_name);
    free(sig_f.arg_types);
    free(sig_g.fct_name);
    free(sig_g.arg_types);

}



int main()
{

    test_normal();

    test_delete_all_for_sock();

    test_round_robin();
}

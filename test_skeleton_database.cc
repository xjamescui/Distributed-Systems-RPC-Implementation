#include "server_function_skels.h"
#include "SkeletonDatabase.h"
#include "defines.h"
#include <assert.h>
#include <iostream>

using namespace std;

SKEL_RECORD skel_f0, skel_f1, skel_f2, skel_f3, skel_f4;

int main()
{

    /* prepare server functions' signatures */
    int count0 = 3;
    int count1 = 5;
    int count2 = 3;
    int count3 = 1;
    int count4 = 1;
    int argTypes0[count0 + 1];
    int argTypes1[count1 + 1];
    int argTypes2[count2 + 1];
    int argTypes3[count3 + 1];
    int argTypes4[count4 + 1];

    argTypes0[0] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
    argTypes0[1] = (1 << ARG_INPUT) | (ARG_INT << 16);
    argTypes0[2] = (1 << ARG_INPUT) | (ARG_INT << 16);
    argTypes0[3] = 0;

    argTypes1[0] = (1 << ARG_OUTPUT) | (ARG_LONG << 16);
    argTypes1[1] = (1 << ARG_INPUT) | (ARG_CHAR << 16);
    argTypes1[2] = (1 << ARG_INPUT) | (ARG_SHORT << 16);
    argTypes1[3] = (1 << ARG_INPUT) | (ARG_INT << 16);
    argTypes1[4] = (1 << ARG_INPUT) | (ARG_LONG << 16);
    argTypes1[5] = 0;

    /*
     * the length in argTypes2[0] doesn't have to be 100,
     * the server doesn't know the actual length of this argument
     */
    argTypes2[0] = (1 << ARG_OUTPUT) | (ARG_CHAR << 16) | 100;
    argTypes2[1] = (1 << ARG_INPUT) | (ARG_FLOAT << 16);
    argTypes2[2] = (1 << ARG_INPUT) | (ARG_DOUBLE << 16);
    argTypes2[3] = 0;

    /*
     * f3 takes an array of long.
     */
    argTypes3[0] = (1 << ARG_OUTPUT) | (1 << ARG_INPUT) | (ARG_LONG << 16) | 11;
    argTypes3[1] = 0;

    /* same here, 28 is the exact length of the parameter */
    argTypes4[0] = (1 << ARG_INPUT) | (ARG_CHAR << 16) | 28;
    argTypes4[1] = 0;

    skel_f0.fct_name = (char *)"f0";
    skel_f0.arg_types = argTypes0;
    skel_f0.skel = *f0_Skel;

    skel_f1.fct_name = (char *)"f1";
    skel_f1.arg_types = argTypes1;
    skel_f1.skel = *f1_Skel;

    skel_f2.fct_name = (char *)"f2";
    skel_f2.arg_types = argTypes2;
    skel_f2.skel = *f2_Skel;

    skel_f3.fct_name = (char *)"f3";
    skel_f3.arg_types = argTypes3;
    skel_f3.skel = *f3_Skel;

    skel_f4.fct_name = (char *)"f4";
    skel_f4.arg_types = argTypes4;
    skel_f4.skel = *f4_Skel;


    // test starts
    skeleton query_skel;
    SkeletonDatabase* db = new SkeletonDatabase();

    cout << "TEST: insert into empty db" << endl;
    assert(db->db_size() == 0);
    assert(db->db_put(skel_f0) == RECORD_PUT_SUCCESS);
    assert(db->db_size() == 1);
    assert(db->db_put(skel_f0) == RECORD_PUT_DUPLICATE);
    assert(db->db_size() == 1);


    cout << "TEST: delete existing" << endl;
    assert(db->db_delete(skel_f0) == RECORD_DELETE_SUCCESS);
    assert(db->db_size() == 0);

    cout << "TEST: delete on empty db" << endl;
    assert(db->db_delete(skel_f0) == RECORD_DELETE_FAIL);

    assert(db->db_put(skel_f0) == RECORD_PUT_SUCCESS);
    assert(db->db_size() == 1);

    assert(db->db_put(skel_f1) == RECORD_PUT_SUCCESS);
    assert(db->db_size() == 2);

    assert(db->db_put(skel_f2) == RECORD_PUT_SUCCESS);
    assert(db->db_size() == 3);

    cout << "TEST: get" << endl;

    assert(db->db_get(&query_skel, (char *)"f0", argTypes0) == RECORD_FOUND);
    assert(query_skel == f0_Skel);

    assert(db->db_get(&query_skel, (char *)"f2", argTypes0) == RECORD_NOT_FOUND);
    assert(query_skel == NULL);

    assert(db->db_get(&query_skel, (char *)"f4", argTypes4) == RECORD_NOT_FOUND);
    assert(query_skel == NULL);

    assert(db->db_put(skel_f4) == RECORD_PUT_SUCCESS);
    assert(db->db_size() == 4);

    assert(db->db_get(&query_skel, (char *)"f4", argTypes4) == RECORD_FOUND);
    assert(query_skel == f4_Skel);

    cout << "ALL TESTS PASS!" << endl;

    delete db;

} // main


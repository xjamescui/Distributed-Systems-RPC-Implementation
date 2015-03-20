#ifndef SERVER_DATABASE_H
#define SERVER_DATABASE_H

#include "rpc.h"
#include "defines.h"
#include <list>


typedef struct _SKEL_RECORD_ {
    char *fct_name;
    int *arg_types;
    skeleton skel;
} SKEL_RECORD;

class SkeletonDatabase
{
private:
    std::list<SKEL_RECORD> _db;

    void find_record(SKEL_RECORD **found, char* fct_name, int* arg_types);
    bool same_signature(const SKEL_RECORD* record, char* fct_name, int* arg_types);

public:
    SkeletonDatabase();
    int db_put(SKEL_RECORD record);

    // get the skeleton associated with fct_name and arg_types
    int db_get(skeleton *skel, char* fc_name, int* arg_types);

    int db_delete(SKEL_RECORD record);

    // print out the contents of the database
    void db_print();

    // return the size of the database
    int db_size();

};

#endif // SERVER_DATBASE_H

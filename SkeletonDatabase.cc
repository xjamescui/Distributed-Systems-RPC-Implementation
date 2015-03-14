#include <string.h>
#include "SkeletonDatabase.h"
#include "debug.h"
#include "helper.h"

using namespace std;

SkeletonDatabase::SkeletonDatabase(){} // constructor

/**
 * Insert a record into the database if it currently does not already exist in the db
 *
 * Returns:
 * RECORD_PUT_SUCCESS
 * RECORD_PUT_DUPLICATE
 */
int SkeletonDatabase::db_put(SKEL_RECORD record){

    if (!this->_db.empty()){
        // check for duplicate
        SKEL_RECORD* duplicate_record = NULL;
        this->find_record(&duplicate_record, record.fct_name, record.arg_types);
        if (duplicate_record != NULL){
            DEBUG("duplicate during db_put");
            return RECORD_PUT_DUPLICATE;
        }

    }

    // insert
    this->_db.push_back(record);
    return RECORD_PUT_SUCCESS;

} // db_put


/**
 * Retrieve the skeleton that matches the given function name and argument types
 *
 * Returns:
 * RECORD_FOUND
 * RECORD_NOT_FOUND
 */
int SkeletonDatabase::db_get(skeleton *skel, char* fct_name, int* arg_types) {

    if (this->_db.empty()) return RECORD_NOT_FOUND;

    // find
    SKEL_RECORD* found_record = NULL;
    this->find_record(&found_record, fct_name, arg_types);
    if (found_record == NULL){
        *skel = NULL;
        return RECORD_NOT_FOUND;
    }

    if (skel != NULL) *skel = found_record->skel;
    return RECORD_FOUND;
} // db_get


/**
 * Returns:
 * RECORD_DELETE_SUCCESS
 * RECORD_DELETE_FAIL
 */
int SkeletonDatabase::db_delete(SKEL_RECORD record) {
    int deleteOpCode = RECORD_DELETE_FAIL;
    for (list<SKEL_RECORD>::iterator it=this->_db.begin(); it != this->_db.end(); ++it) {
        if (this->same_signature(&(*it), record.fct_name, record.arg_types)) {
            this->_db.erase(it);
            deleteOpCode = RECORD_DELETE_SUCCESS;
            break;
        }
    }
    return deleteOpCode;
} // db_delete


/**
 * Prints out the contents of the database
 */
void SkeletonDatabase::db_print(){
    int index = 0;
    unsigned int arg_types_len;
    DEBUG("Printing.. Skel DB size: %d\n ", (int)this->_db.size());
    for (list<SKEL_RECORD>::iterator it = this->_db.begin(); it != this->_db.end(); ++it) {
        arg_types_len = arg_types_length(it->arg_types);
        DEBUG("Record %d: %s: ", index, it->fct_name, arg_types_len);
        for (unsigned int i = 0; i < arg_types_len; i++) {
            DEBUG("%d ", it->arg_types[i]);
        }
        DEBUG(": ");
        DEBUG("%lu\n", (long)(it->skel));
        index++;
    }
} 

int SkeletonDatabase::db_size(){
    return this->_db.size();
}

/**
 * Finds a record in the database that matches the function name and arg types
 */
void SkeletonDatabase::find_record( SKEL_RECORD **found, char* fct_name, int* arg_types){

    for (list<SKEL_RECORD>::iterator it=this->_db.begin(); it != this->_db.end(); ++it) {
        if (this->same_signature(&(*it), fct_name, arg_types)) {
            *found = &(*it);
            break;
        }
    }
}

/**
 * Checks if a record has the wanted signature (function name and argument types)
 */
bool SkeletonDatabase::same_signature(const SKEL_RECORD *record, char* fct_name, int* arg_types) {

    // compare function names
    unsigned int name_len = strlen(record->fct_name);
    if (name_len != strlen(fct_name)) {
        return false;
    } else{
        for (unsigned int i = 0; i < name_len; i++) {
            if (record->fct_name[i] != fct_name[i]) {
                return false;
            }
        }
    }

    // compare arguments
    unsigned int arg_types_len = arg_types_length(arg_types);
    unsigned int record_arg_types_len = arg_types_length(record->arg_types);
    if (record_arg_types_len != arg_types_len) {
        return false;
    } else {
        for (unsigned int i = 0; i < arg_types_len; i++){
            if (record->arg_types[i] != arg_types[i]){
                return false;
            }
        }
    }
    return true;
}

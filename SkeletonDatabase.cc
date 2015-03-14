#include <string.h>
#include "SkeletonDatabase.h"
#include "debug.h"
#include "helper.h"

using namespace std;

SkeletonDatabase::SkeletonDatabase(){}
/**
 * Returns:
 * RECORD_PUT_SUCCESS
 * RECORD_PUT_DUPLICATE
 */
int SkeletonDatabase::db_put(SKEL_RECORD record){

    if (!this->db.empty()){
        // check for duplicate
        SKEL_RECORD* duplicate_record = NULL;
        this->find_record(&duplicate_record, record.fct_name, record.arg_types);
        if (duplicate_record != NULL){
            DEBUG("duplicate during db_put");
            return RECORD_PUT_DUPLICATE;
        }

    }

    // insert
    this->db.push_back(record);
    return RECORD_PUT_SUCCESS;

} // db_put


/**
 * Returns:
 * RECORD_FOUND
 * RECORD_NOT_FOUND
 */
int SkeletonDatabase::db_get(skeleton *skel, char* fct_name, int* arg_types) {

    if (this->db.empty()) return RECORD_NOT_FOUND;

    // find
    SKEL_RECORD* found_record = NULL;
    this->find_record(&found_record, fct_name, arg_types);
    if (found_record == NULL) return RECORD_NOT_FOUND;

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
    for (list<SKEL_RECORD>::iterator it=this->db.begin(); it != this->db.end(); ++it) {
        if (this->same_signature(&(*it), record.fct_name, record.arg_types, record.arg_types_len)) {
            this->db.erase(it);
            deleteOpCode = RECORD_DELETE_SUCCESS;
            break;
        }
    }
    return deleteOpCode;
} // db_delete


void SkeletonDatabase::db_print(){
    int index = 0;
    DEBUG("Printing.. Skel DB size: %d\n ", (int)this->db.size());
    for (list<SKEL_RECORD>::iterator it = this->db.begin(); it != this->db.end(); ++it) {
        DEBUG("Record %d: %s: %d: ", index, it->fct_name, it->arg_types_len);
        for (unsigned int i = 0; i < it->arg_types_len; i++) {
            DEBUG("%d ", it->arg_types[i]);
        }
        DEBUG(": ");
        DEBUG("%lu\n", (long)(it->skel));
        index++;
    }
} 

void SkeletonDatabase::find_record( SKEL_RECORD **found, char* fct_name, int* arg_types){

    unsigned int arg_types_len = arg_types_length(arg_types);
    for (list<SKEL_RECORD>::iterator it=this->db.begin(); it != this->db.end(); ++it) {
        if (this->same_signature(&(*it), fct_name, arg_types, arg_types_len)) {
            **found = *it;
            break;
        }
    }
}

bool SkeletonDatabase::same_signature(const SKEL_RECORD *record, char* fct_name, int* arg_types, unsigned int arg_types_len) {

    // compare function names
    unsigned int name_len = strlen(record->fct_name);
    if (name_len != strlen(fct_name)) {
        DEBUG("strlen not equal");
        return false;
    } else{
        for (unsigned int i = 0; i < name_len; i++) {
            if (record->fct_name[i] != fct_name[i]) {
                DEBUG("names not equal");
                return false;
            }
        }
    }

    // compare arguments
    if (record->arg_types_len != arg_types_len) {
        DEBUG("arg types len not equal");
        return false;
    } else {
        for (unsigned int i = 0; i < arg_types_len; i++){
            if (record->arg_types[i] != arg_types[i]){
                DEBUG("arg types not equal");
                return false;
            }
        }
    }

    return true;
}

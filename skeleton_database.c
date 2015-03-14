#include "skeleton_database.h"
#include "stdbool.h"
#include "debug.h"
#include <string.h>

SKEL_RECORD* skel_db_head = NULL;

bool is_same_sig(const SKEL_RECORD *s1, const SKEL_RECORD *s2);
bool get_skel_record(SKEL_RECORD** result, SKEL_RECORD target);


/**
 * insert a new skeleton record into db
 *
 * returns:
 * RECORD_PUT_SUCCESS
 * or RECORD_PUT_DUPLICATE
 *
 */
int skel_db_put(SKEL_RECORD record){

    DEBUG("skel_db_put");

    int ret_code = RECORD_PUT_SUCCESS;
    SKEL_RECORD* found = NULL;
    // check for duplicate
    if (get_skel_record(&found, record)){
      // already exists, just give warning
      return RECORD_PUT_DUPLICATE;
      

    }

    // insert

}// skel_db_put

// remove the specified skeleton record in db
int skel_db_delete(SKEL_RECORD sig){}

// find skeleton using given fct_name and arg_types, store result in skel
int skel_db_get(skeleton *skel, char* fct_name, int *arg_types){
    // TODO
} // skel_db_get

// print out the contents of the database
int skel_db_print(){
  // TODO
}


/**
 * check function name and arg types are the same between two skeleton records
 */
bool is_same_sig(const SKEL_RECORD *s1, const SKEL_RECORD *s2) {

    // check fct_name
    if (strlen(s1->fct_name) != strlen(s2->fct_name)) {
        return false;
    } else {
        for ( unsigned int i = 0 ; i < strlen(s1->fct_name) ; i+= 1 ) {
            if ( s1->fct_name[i] != s2->fct_name[i] ) {
                return false;
            }
        }
    }

    // check arg_types_len
    if ( s1->arg_types_len != s2->arg_types_len) return false;

    // check arg_types
    for ( unsigned int i = 0 ; i < s1->arg_types_len ; i += 1 ) {
        if ( s1->arg_types[i] != s2->arg_types[i] ) {
            return false;
        }
    }

    return true;
}

bool get_skel_record(SKEL_RECORD** result, SKEL_RECORD target ) {
    SKEL_RECORD* temp = skel_db_head;
    while ( temp != NULL ) {
        if ( is_same_sig(temp, &target) ) {
            *result = temp;
            return true;
        }
        temp = temp->next;
    }
    return false;
}

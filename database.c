#include "database.h"
#include "debug.h"
#include "defines.h"

#include <unistd.h>
#include <string.h>
#include <stdlib.h>


// #define DEBUG(args ...) do { } while (0)



/**
 * root node
 *
 *
 *
 */

DB_NODE* g_db_nodes_root = NULL;



/****
 * database logic
 *
 *
 *
 *
 *
 *
 */
bool is_same_sig(const SIGNATURE &a, const SIGNATURE &b) {
    // check fct_name_len
    if ( a.fct_name_len != b.fct_name_len ) {
        return false;
    }
    // check fct_name
    for ( unsigned int i = 0 ; i < a.fct_name_len ; i+= 1 ) {
        if ( a.fct_name[i] != b.fct_name[i] ) {
            return false;
        }
    }
    // check arg_types_len
    if ( a.arg_types_len != b.arg_types_len ) {
        return false;
    }
    // check arg_types
    for ( unsigned int i = 0 ; i < a.arg_types_len ; i += 1 ) {
        if ( a.arg_types[i] != b.arg_types[i] ) {
            return false;
        }
    }
    return true;
}

bool is_same_host(const HOST* const a, const HOST* const b) {
    return (a->ip == b->ip) && (a->port == b->port);
}

bool get_db_node(DB_NODE** node, SIGNATURE sig) {
    DB_NODE* temp = g_db_nodes_root;
    while ( temp != NULL ) {
        if ( is_same_sig(temp->sig,sig) ) {
            *node = temp;
            return true;
        }
        temp = temp->next;
    }
    return false;
}

/**
 * put
 *
 * returns:
 * SIGNATURE_PUT_SUCCESS
 * SIGNATURE_PUT_DUPLICATE if duplicate
 *
 *
 *
 */
int db_put(unsigned int ip, unsigned int port, SIGNATURE sig) {

    DEBUG("db_put");

    int ret_code = SIGNATURE_PUT_SUCCESS;

    // find the sig
    DB_NODE* found_node = NULL;
    if ( get_db_node(&found_node,sig) == true ) {
        // found a node
        if ( db_delete_host(ip,port,sig) == DELETE_HOST_SUCCESS ) {
            // found sig and removed it
            ret_code = SIGNATURE_PUT_DUPLICATE;
        }
    } else {
        // can't find : make a new node
        found_node = (DB_NODE*)malloc(sizeof(DB_NODE));

        // copy sig
        found_node->sig.fct_name_len = sig.fct_name_len;
        found_node->sig.fct_name = (char*)malloc(sig.fct_name_len);
        memcpy(found_node->sig.fct_name,sig.fct_name,sig.fct_name_len);
        found_node->sig.arg_types_len = sig.arg_types_len;
        found_node->sig.arg_types = (int*)malloc(4*sig.arg_types_len);
        memcpy(found_node->sig.arg_types,sig.arg_types,4*sig.arg_types_len);

        // set hosts_root
        found_node->hosts_root = NULL;

        // set next
        found_node->next = NULL;

        // append node in the db
        if ( g_db_nodes_root == NULL ) {
            g_db_nodes_root = found_node;
        } else {
            DB_NODE* last_node = g_db_nodes_root;
            while ( last_node->next != 0 ) {
                last_node = last_node->next;
            }
            last_node->next = found_node;
        }
    }

    // append host
    HOST* new_host = (HOST*)malloc(sizeof(HOST));
    new_host->ip = ip;
    new_host->port = port;
    new_host->next = NULL;
    if ( found_node->hosts_root == NULL ) {
        // if node is empty
        found_node->hosts_root = new_host;
    } else {
        // if node not empty
        HOST* last_host = found_node->hosts_root;
        while ( last_host->next != 0 ) {
            last_host = last_host->next;
        }
        last_host->next = new_host;
    }

    return ret_code;
}




/**
 * get
 *
 * 
 *
 *
 *
 *
 */
int db_get(unsigned int *ip, unsigned int *port, SIGNATURE sig) {

    DEBUG("db_get");

    // find the sig
    DB_NODE* found_node = NULL;
    if ( get_db_node(&found_node,sig) == false ) {
        return SIGNATURE_NOT_FOUND;
    }

    // return the first ip
    HOST* first_host = found_node->hosts_root;
    if ( first_host == NULL ) {
        *ip = 0;
        *port = 0;
        return SIGNATURE_HAS_NO_HOSTS;
    }
    *ip = first_host->ip;
    *port = first_host->port;

    // put head node to end
    // HOST* last_host = host;
    // while( last_host->next != NULL ) {
    //     last_host = last_host->next;
    // }
    // if ( last_host != host ) {
    //     found_node->hosts_root = found_node->hosts_root->next;
    //     last_host->next = host;
    //     host->next = NULL;
    // }

    // put the HOST of for every sig to the end
    DB_NODE* temp_node = g_db_nodes_root;
    while ( temp_node != NULL ) {
        HOST* temp_first_host = temp_node->hosts_root;
        if ( temp_first_host != NULL ) {
            if ( is_same_host(temp_first_host,first_host) ) {
                // find last
                HOST* temp_last = temp_first_host;
                while ( temp_last->next != NULL ) {
                    temp_last = temp_last->next;
                }
                if ( temp_last != temp_first_host ) {
                    temp_last->next = temp_first_host;
                    temp_node->hosts_root = temp_first_host->next;
                    temp_first_host->next = NULL;
                }
            }
        }
        temp_node = temp_node->next;
    }

    return SIGNATURE_FOUND;
}


/**
 * delete host
 *
 *
 *
 *
 *
 *
 */
int db_delete_host(unsigned int ip, unsigned int port, SIGNATURE sig) {

    DEBUG("db_delete_host");

    // find the sig
    DB_NODE* found_node = NULL;
    if ( get_db_node(&found_node,sig) == false ) {
        return SIGNATURE_NOT_FOUND;
    }

    // look for the host
    HOST* host = found_node->hosts_root;
    while ( host != NULL ) {
        if ( host->ip == ip &&
             host->port == port ) {
            // found it
            if ( host == found_node->hosts_root ) {
                // if it's first
                found_node->hosts_root = host->next;
            } else {
                // find the previous one
                HOST* prev = found_node->hosts_root;
                while ( prev->next != host ) {
                    prev = prev->next;
                }
                prev->next = host->next;

            }
            free(host);
            break;
        }
        host = host->next;
    }

    if ( host == NULL ) {
        return DELETE_HOST_NOT_FOUND;
    } else {
        return DELETE_HOST_SUCCESS;
    }

}





/**
 * drop
 *
 *
 *
 *
 *
 *
 */
int db_drop() {
    DEBUG("db_drop");
    DB_NODE* temp_node = g_db_nodes_root;
    HOST* temp_host = NULL;
    while( temp_node != NULL ) {
        g_db_nodes_root = temp_node->next;
        free(temp_node->sig.fct_name);
        free(temp_node->sig.arg_types);
        temp_host = temp_node->hosts_root;
        while( temp_host != NULL ) {
            temp_node->hosts_root = temp_host->next;
            free(temp_host);
            temp_host = temp_node->hosts_root;
        }
        free(temp_node);
        temp_node = g_db_nodes_root;
    }
    return 0;
}

/**
 * size
 *
 *
 *
 *
 *
 *
 */
int db_size(unsigned int *size) {

    *size = 0 ;

    DB_NODE* temp_node = g_db_nodes_root;
    while( temp_node != NULL ) {
        HOST* temp_host = temp_node->hosts_root;
        while ( temp_host != NULL ) {
            *size += 1;
            temp_host = temp_host->next;
        }
        temp_node = temp_node->next;
    }

    return 0;
}


/**
 * print
 *
 *
 *
 *
 *
 *
 */
int db_print() {

    DEBUG("printing db start ====");

    DB_NODE* temp_node = g_db_nodes_root;
    while ( temp_node != NULL ) {
        DEBUG("N:%s",temp_node->sig.fct_name);
        for ( unsigned int i = 0 ; i < temp_node->sig.arg_types_len ; i += 1 ) {
            DEBUG("Arg:%x %x %x",
                (temp_node->sig.arg_types[i] >> 24) & 0xff,
                (temp_node->sig.arg_types[i] >> 16) & 0xff,
                (temp_node->sig.arg_types[i] >> 0) & 0xffff);
        }
        HOST* temp_host = temp_node->hosts_root;
        while ( temp_host != NULL ) {
            DEBUG("    H:%x.%x.%x.%x %x",
                    (temp_host->ip >> 24 ) & 0xff,
                    (temp_host->ip >> 16 ) & 0xff,
                    (temp_host->ip >> 8 ) & 0xff,
                    (temp_host->ip >> 0 ) & 0xff,
                    (temp_host->port >> 0 ) & 0xffff
                    );
            temp_host = temp_host->next;
        }
        temp_node = temp_node->next;
    }

    DEBUG("printing db end ====");

    return 0;
}
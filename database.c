#include "database.h"
#include "defines.h"

#include <unistd.h>
#include <string.h>
#include <stdlib.h>

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
bool is_same_sig(SIGNATURE a, SIGNATURE b) {
    // check fct_name
    if ( strcmp(a.fct_name,b.fct_name) != 0 ) {
        return false;
    }
    // check num_args
    if ( a.num_args != b.num_args ) {
        return false;
    }
    // check arg_types
    for ( unsigned int i = 0 ; i < a.num_args ; i += 1 ) {
        if ( a.arg_types[i] != b.arg_types[i] ) {
            return false;
        }
    }
    return true;
}



/**
 * put
 *
 *
 *
 *
 *
 *
 */
int db_put(unsigned int ip, unsigned int port, SIGNATURE sig) {

    // delete it if it exists, and re-add it
    if ( db_get(&ip,&port,sig) >= 0 ) {
        db_delete_host(ip,port,sig);
        return db_put(ip,port,sig);
    }

    // look to see if the node is there
    DB_NODE* found_node = g_db_nodes_root;
    while(found_node != NULL){
        if ( is_same_sig(found_node->sig,sig) ) {
            break;
        }
        found_node = found_node->next;
    }

    // can't find : make a new node
    if ( found_node == NULL ) {
        found_node = (DB_NODE*)malloc(sizeof(DB_NODE));
        found_node->sig = sig;
        found_node->next = NULL;
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

    // append to the end
    HOST* new_host = (HOST*)malloc(sizeof(HOST));
    new_host->ip = ip;
    new_host->port = port;
    new_host->next = NULL;
    if ( found_node->hosts_root == NULL ) {
        found_node->hosts_root = new_host;
    } else {
        HOST* last_host = found_node->hosts_root;
        while ( last_host->next != 0 ) {
            last_host = last_host->next;
        }
        last_host->next = new_host;
    }

    return SIGNATURE_PUT_SUCCESS;
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

    // look for sig
    DB_NODE* found_node = g_db_nodes_root;
    while( found_node != NULL ) {
        if ( is_same_sig(found_node->sig,sig) ) {
            break;
        }
        found_node = found_node->next;
    }

    // if can't find
    if ( found_node == NULL ) {
        return SIGNATURE_NOT_FOUND;
    }

    // return the first ip
    HOST* host = found_node->hosts_root;
    if ( host == NULL ) {
        *ip = 0;
        *port = 0;
        return SIGNATURE_HAS_NO_HOSTS;
    }
    *ip = host->ip;
    *port = host->port;

    // put head node to end
    HOST* last_host = host;
    while( last_host->next != NULL ) {
        last_host = last_host->next;
    }
    if ( last_host != host ) {
        found_node->hosts_root = found_node->hosts_root->next;
        last_host->next = host;
        host->next == NULL;
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

    // find the sig
    DB_NODE* found_node = g_db_nodes_root;
    while( found_node != NULL ) {
        if ( is_same_sig(found_node->sig,sig) ) {
            break;
        }
        found_node = found_node->next;
    }

    // if can't find
    if ( found_node == NULL ) {
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
                // find the prvious one
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
    return DELETE_HOST_SUCCESS;
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
        temp_node = g_db_nodes_root;
    }
    return 0;
}



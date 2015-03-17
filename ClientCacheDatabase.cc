#include <stddef.h> // NULL
#include <stdio.h>
#include <stdlib.h>

#include "ClientCacheDatabase.h"

// init singleton instance to null
ClientCacheDatabase* ClientCacheDatabase::m_pInstance = NULL;

// Instance()
ClientCacheDatabase* ClientCacheDatabase::Instance()
{
    // only generate at beginning
    if (m_pInstance == NULL) {
        m_pInstance = new ClientCacheDatabase;
    }

    return m_pInstance;
}

ClientCacheDatabase::ClientCacheDatabase()
{
    g_db_nodes_root = NULL;
    if ( pthread_mutex_init(&m_lock,NULL) < 0 ) {
        fprintf(stderr,"FATAL: ClientCacheDatabase fail to init lock\n");
        exit(1);
    }
};

ClientCacheDatabase::~ClientCacheDatabase()
{
    this->drop();
    pthread_mutex_destroy(&m_lock);
}

int ClientCacheDatabase::put(const HOST &host, SIGNATURE sig)
{
    int opCode;
    pthread_mutex_lock(&m_lock);        // lock
    opCode = db_put(host,sig);          // do the function
    pthread_mutex_unlock(&m_lock);      // unlock
    return opCode;                      // return the opCode

}
int ClientCacheDatabase::get(HOST* host, SIGNATURE sig)
{
    int opCode;
    pthread_mutex_lock(&m_lock);        // lock
    opCode = db_get(host,sig);          // do the function
    pthread_mutex_unlock(&m_lock);      // unlock
    return opCode;                      // return the opCode
}
int ClientCacheDatabase::delete_host(const HOST &host, SIGNATURE sig)
{
    int opCode;
    pthread_mutex_lock(&m_lock);        // lock
    opCode = db_delete_host(host,sig);  // do the function
    pthread_mutex_unlock(&m_lock);      // unlock
    return opCode;                      // return the opCode
}
int ClientCacheDatabase::drop()
{
    int opCode;
    pthread_mutex_lock(&m_lock);        // lock
    opCode = db_drop();                 // do the function
    pthread_mutex_unlock(&m_lock);      // unlock
    return opCode;                      // return the opCode
}

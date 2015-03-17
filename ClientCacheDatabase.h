#ifndef CLIENT_CACHE_DATABASE_H
#define CLIENT_CACHE_DATABASE_H

/**
 * This is just a wrapper to
 * binder database .. as a singleton
 *
 * singleton template reference: http://www.yolinux.com/TUTORIALS/C++Singleton.html
 *
 *
 */

#include <pthread.h>
#include "host_database.h"

class ClientCacheDatabase
{
public:
    // singleton call
    static ClientCacheDatabase* Instance();

public:

    int put(const HOST &host, SIGNATURE sig);
    int get(HOST* host, SIGNATURE sig);
    int delete_host(const HOST &host, SIGNATURE sig);
    int drop();

private:
    // make default functions private
    ClientCacheDatabase();
    ~ClientCacheDatabase();
    ClientCacheDatabase(ClientCacheDatabase const&);
    ClientCacheDatabase& operator=(ClientCacheDatabase const&);

    static ClientCacheDatabase* m_pInstance;
    static pthread_mutex_t m_lock;
};

#endif // CLIENT_CACHE_DATABASE_H
/*

Copyright (C) 2010-2013 KWARC Group <kwarc.info>

This file is part of MathWebSearch.

MathWebSearch is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MathWebSearch is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MathWebSearch.  If not, see <http://www.gnu.org/licenses/>.

*/
/**
 * @brief Implementation of the DBHandle class
 *
 * @author Daniel Hasegan d.hasegan@jacobs-university.de
 * @date 27.III.2013
 */

// System includes
#include <db.h>                        // C API for BerkleyDB
#include <unistd.h>                    // C POSIX misc functions
#include <cstdio>                      // C standard I/O library
#include <cstdlib>                     // C general purpose library
#include <cstring>                     // C string header
#include <string>                      // STL string
#include <cerrno>                      // C errno code header

// Local includes
#include "PageDbHandle.hpp"
#include "MwsDbcMacros.hpp"            // MWS Database default macros
#include "common/utils/DebugMacros.hpp"// MWS Debug Macro Utilities
#include "common/utils/macro_func.h"   // MWS Utility functions

// Namespaces

using namespace std;
using namespace mws;

static int
my_compare_data(DB*        dbp,
                const DBT* a,
                const DBT* b)
{
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_IN;
#endif

    UNUSED(dbp);

    size_t len1;
    size_t len2;
    int    result;

    memcpy (&len1, a->data, sizeof(size_t));
    len1 -= sizeof(size_t);
    memcpy (&len2, b->data, sizeof(size_t));
    len2 -= sizeof(size_t);

    if (len1 < len2)
    {
        result = -1;
    }
    else if (len1 > len2)
    {
        result = 1;
    }
    else
    {
        result = memcmp((char*)a->data + sizeof(size_t),
                        (char*)b->data + sizeof(size_t),
                        len1);
    }

#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_OUT;
#endif
    return result;
}


static int
databaseRemove(const char* file,
               const char* database,
               int         flags)
{
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_IN;
#endif

    DB* localDbHandle;
    int ret;

    // Removing the database file
    db_create(&localDbHandle, NULL, 0);

    ret = localDbHandle->remove(localDbHandle,
                                file,
                                database,
                                flags);
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_OUT;
#endif
    return ret;
}

static int
databaseEnvRemove(const char* dbenvpath,
                  int         flags)
{
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_IN;
#endif

    DB_ENV* localDbEnvHandle;
    int     ret;

    // Removing the database file
    db_env_create(&localDbEnvHandle, 0);

    ret = localDbEnvHandle->remove(localDbEnvHandle,
                                   dbenvpath,
                                   flags);
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_OUT;
#endif
    return ret;
}


PageDbHandle::PageDbHandle() {
}

int PageDbHandle::init(string dbenv_dir)
{
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_IN;
#endif

    int ret = 0;

    // Setting alive instances to the default value
    alive = 0;

    // set DBENV path
    dbenvDir = dbenv_dir;

    // Initializing the mutex
    if ((ret = pthread_mutex_init(&lock,
                                  NULL))
            != 0)
    {
        perror("pthread_mutex_init");
    }
    // Initializing the condition
    else if ((ret = pthread_cond_init(&instanceClosed,
                                      NULL))
             != 0)
    {
        perror("pthread_cond_init");
        pthread_mutex_destroy(&lock);
    }
    // Initializing an environment handle structure
    else if ((ret = db_env_create(&dbEnv,
                                  0))
             != 0)
    {
        fprintf(stderr, "Error while db_env_create\n");
        pthread_cond_destroy(&instanceClosed);
        pthread_mutex_destroy(&lock);
    }
    // Setting the cache size
    else if ((ret = dbEnv->set_cachesize(dbEnv,
                                         0,
                                         MWS_DB_CACHESIZE,
                                         0))
             != 0)
    {
        fprintf(stderr, "Error while setting env cache size\n");
        dbEnv->close(dbEnv, 0);
        pthread_cond_destroy(&instanceClosed);
        pthread_mutex_destroy(&lock);
    }
    // Setting the max mmap size
    else if ((ret = dbEnv->set_mp_mmapsize(dbEnv,
                                           MWS_DB_MMAPSIZE))
             != 0)
    {
        fprintf(stderr, "Error while setting env mmap size\n");
        dbEnv->close(dbEnv, 0);
        pthread_cond_destroy(&instanceClosed);
        pthread_mutex_destroy(&lock);
    }
    // Opening the environment
    else if (( ret = dbEnv->open(dbEnv,
                                 dbenvDir.c_str(),
                                 DB_CREATE | DB_INIT_MPOOL | DB_THREAD,
                                 0))
             != 0)
    {
        fprintf(stderr, "Error while opening env\n");
        dbEnv->close(dbEnv, 0);
        pthread_cond_destroy(&instanceClosed);
        pthread_mutex_destroy(&lock);
    }
    // Initializing a database handle structure
    else if ((ret = db_create(&dbHandle,
                              dbEnv,
                              0))
             != 0)
    {
        fprintf(stderr, "Error while db_create\n");
        dbEnv->close(dbEnv, 0);
        pthread_cond_destroy(&instanceClosed);
        pthread_mutex_destroy(&lock);
    }
    // Setting the duplicate key/value pair support
    else if ((ret = dbHandle->set_flags(dbHandle,
                                        DB_DUPSORT))
             != 0)
    {
        fprintf(stderr, "Error while setting DB_DUPSORT flag\n");
        dbHandle->close(dbHandle, 0);
        dbEnv->close(dbEnv, 0);
        pthread_cond_destroy(&instanceClosed);
        pthread_mutex_destroy(&lock);
    }
    // Setting the (duplicate) data compare-function
    else if ((ret = dbHandle->set_dup_compare(dbHandle,
                                              my_compare_data))
             != 0)
    {
        fprintf(stderr, "Error while setting DB_DUPSORT flag\n");
        dbHandle->close(dbHandle, 0);
        dbEnv->close(dbEnv, 0);
        pthread_cond_destroy(&instanceClosed);
        pthread_mutex_destroy(&lock);
    }
    // Opening the database
    else if ((ret = dbHandle->open(dbHandle,      // DB structure pointer
                                   NULL,          // Transaction pointer
                                   MWS_DB_FILE,   // On-disk db file
                                   MWS_DB_NAME,   // logical db name (opt)
                                   DB_BTREE,      // db access method
                                   DB_CREATE | DB_THREAD,  // flags
                                   0))            // File mode (0 = defaults)
             != 0)
    {
        fprintf(stderr, "Error while opening database!\n");
        dbHandle->close(dbHandle, 0);
        dbEnv->close(dbEnv, 0);
        pthread_cond_destroy(&instanceClosed);
        pthread_mutex_destroy(&lock);
    }

#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_OUT;
#endif
    return ret;
}

void PageDbHandle::clean()
{

#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_IN;
#endif
    int ret;

    this->close();

    // Removing database file
    string dbfile = dbenvDir + "/" + (string) MWS_DB_FILE;
    ret = databaseRemove(dbfile.c_str(),
                         NULL,
                         0);
    if (ret == ENOENT)
    {
        ret = databaseRemove(MWS_DB_FILE,
                             NULL,
                             0);
    }
    if (ret != 0)
    {
        perror("Removing database");
    }

    // Removing database environment files
    ret = databaseEnvRemove(dbenvDir.c_str(), 0);
    if (ret != 0)
    {
        perror("Removing database environment");
    }


#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_OUT;
#endif
}

void PageDbHandle::close() {
    pthread_mutex_lock(&lock);
    while (alive)
    {
        pthread_cond_wait(&instanceClosed,
                          &lock);
    }
    pthread_mutex_unlock(&lock);

    pthread_cond_destroy(&instanceClosed);

    pthread_mutex_destroy(&lock);

    if (dbHandle != NULL)
        dbHandle->close(dbHandle, 0);

    if (dbEnv != NULL)
        dbEnv->close(dbEnv, 0);
}

PageDbConn* PageDbHandle::createConnection() {

    PageDbConn* conn = new PageDbConn(this);

    return conn;
}

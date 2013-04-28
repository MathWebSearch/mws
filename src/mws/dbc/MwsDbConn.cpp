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
  * @brief File containing the implementation of the MwsDatabase Connection
  * class.
  * @file MwsDbConn.cpp
  * @author Corneliu-Claudiu Prodescu
  * @date 17 May 2011
  *
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

#include "MwsDbConn.hpp"               // MWS Database connection header file
#include "MwsDbData.hpp"               // MWS Database data encoder/decoder
#include "MwsDbcMacros.hpp"            // MWS Database default macros
#include "common/utils/DebugMacros.hpp"// MWS Debug Macro Utilities
#include "common/utils/macro_func.h"   // MWS Utility functions


// Namespaces

using namespace std;
using namespace mws;

// Static variables

string          mws::MwsDbConn::dbenvDir;
DB_ENV*         mws::MwsDbConn::dbEnv;
DB*             mws::MwsDbConn::dbHandle;
pthread_mutex_t mws::MwsDbConn::lock;
pthread_cond_t  mws::MwsDbConn::instanceClosed;
unsigned int    mws::MwsDbConn::alive;

// Local functions

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


mws::MwsDbConn::MwsDbConn()
{
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_IN;
#endif

    // Acquiring lock and signaling that a new instance is alive
    pthread_mutex_lock(&lock);
    alive++;
    pthread_mutex_unlock(&lock);

#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_OUT;
#endif
}


mws::MwsDbConn::~MwsDbConn()
{
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_IN;
#endif

    pthread_mutex_lock(&lock);
    alive--;
    if (0 == alive)
        pthread_cond_signal(&instanceClosed);
    pthread_mutex_unlock(&lock);

#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_OUT;
#endif
}


int
mws::MwsDbConn::init(string dbenv_dir)
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


void
mws::MwsDbConn::clean()
{
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_IN;
#endif

    int ret;

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


int
mws::MwsDbConn::insert(const char*        url_uuid,
                       const char*        xpath,
                       unsigned long long id)
{
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_IN;
#endif

    MwsDbData* dataEnc;
    DBT key, data;
    int ret;
    int result;

    result = 0;

    // Preparing key
    memset(&key, 0, sizeof(DBT));
    key.data = &id;
    key.size = sizeof(unsigned long long);

    // Allocating an encoded data object
    dataEnc = new MwsDbData(url_uuid, xpath);

    // Preparing data
    memset(&data, 0, sizeof(DBT));
    data.data = dataEnc->getSerialized();
    data.size = dataEnc->getSize();

    ret = dbHandle->put(dbHandle,
                        NULL,
                        &key,
                        &data,
                        DB_NODUPDATA);  // No update if key/data exists

    delete dataEnc;

    // If insert was successfull, increment result
    if (ret == 0)
    {
        result++;
    }

#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_OUT;
#endif
    return result;
}


int
mws::MwsDbConn::query(unsigned long long id,
                      unsigned int       limitMin,
                      unsigned int       limitSize,
                      MwsAnswset*        resultAnswset)
{
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_IN;
#endif

    vector<string>::iterator it;
    MwsDbData* dataDec;
    MwsAnsw*   answer;
    DBC* cursorp;
    DBT key, data;
    int ret;
    unsigned int found = 0;

    // Getting a cursor
    dbHandle->cursor(dbHandle, NULL, &cursorp, 0);

    // Setting up the query data
    // Preparing key
    memset(&key, 0, sizeof(DBT));
    key.data = &id;
    key.size = sizeof(unsigned long long);
    // Initializing the data
    memset(&data, 0, sizeof(DBT));

    // Querying
    ret = cursorp->get(cursorp, &key, &data, DB_SET);
    while (ret != DB_NOTFOUND)
    {
        if (limitMin <= found && found < limitMin + limitSize)
        {
            // Adding the solution to the answer set
            // Decoding the data
            dataDec = new MwsDbData((char*)data.data, data.size);
            // Creating an answer
            answer = new MwsAnsw(dataDec->getUrlUuid(),
                                 dataDec->getXpath());
            // Computing the substitutions
            answer->subst.qvarXpaths.reserve(resultAnswset->qvarXpaths.size());
            for (it  = resultAnswset->qvarXpaths.begin();
                 it != resultAnswset->qvarXpaths.end();
                 it ++)
            {
                answer->subst.qvarXpaths.push_back(dataDec->getXpath() + *it);
            }
            // Adding the answer
            resultAnswset->answers.push_back(answer);

            delete dataDec;
        }
        ret = cursorp->get(cursorp, &key, &data, DB_NEXT_DUP);
        found++;
    }

    if (cursorp != NULL)
        cursorp->close(cursorp);

#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_OUT;
#endif

    if (found < limitMin)
        return 0;

    return found - limitMin;
}

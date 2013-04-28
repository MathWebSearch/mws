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
 * @file PageDbConn.cpp
 * @brief Implementation of the PageDbConn class
 *
 * @author Daniel Hasegan <d.hasegan@jacobs-university.de>
 * @date 13.IV.2013
 */

// System includes
#include <unistd.h>                    // C POSIX misc functions
#include <cstdio>                      // C standard I/O library
#include <cstdlib>                     // C general purpose library
#include <cstring>                     // C string header
#include <string>                      // STL string
#include <pthread.h>

// Local includes
#include "PageDbHandle.hpp"
#include "PageDbConn.hpp"
#include "SerializePage.hpp"
#include "common/utils/DebugMacros.hpp"
#include "common/utils/macro_func.h"
#include "mws/types/MwsAnswset.hpp"
#include "mws/types/MwsAnsw.hpp"

// Namespaces
using namespace mws;
using namespace std;


PageDbConn::PageDbConn(PageDbHandle *aHandle) {

#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_IN;
#endif

    handle = aHandle;

    // Acquiring lock and signaling that a new instance is alive
    pthread_mutex_lock(&(handle->lock));
    handle->alive++;
    pthread_mutex_unlock(&(handle->lock));

#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_OUT;
#endif

}

PageDbConn::~PageDbConn() {
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_IN;
#endif

    pthread_mutex_lock(&(handle->lock));
    handle->alive--;
    if (0 == handle->alive)
        pthread_cond_signal(&(handle->instanceClosed));
    pthread_mutex_unlock(&(handle->lock));

#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_OUT;
#endif
}


int
PageDbConn::insert(const char*        url_uuid,
                   const char*        xpath,
                   unsigned long long id)
{
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_IN;
#endif

    DBT key, data;
    int ret;
    int result;

    result = 0;

    // Preparing key
    memset(&key, 0, sizeof(DBT));
    key.data = &id;
    key.size = sizeof(unsigned long long);

    // Create Page with the Url Dictionary
    PageData page(&handle->url_dict,url_uuid,xpath);
    // Serialize the Page Data
    SerializePage ser(page.getUrl_id(),page.getUrl_fragid(),page.getXpath());
    // Preparing data
    memset(&data, 0, sizeof(DBT));
    data.data = ser.getSerialized();
    data.size = ser.getSize();

    ret = handle->dbHandle->put(handle->dbHandle,
                                NULL,
                                &key,
                                &data,
                                DB_NODUPDATA);  // No update if key/data exists

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
PageDbConn::query(unsigned long long id,
                  unsigned int       limitMin,
                  unsigned int       limitSize,
                  MwsAnswset*        resultAnswset)
{

#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_IN;
#endif

    vector<string>::iterator it;
    MwsAnsw*   answer;
    DBC* cursorp;
    DBT key, data;
    int ret;
    unsigned int found = 0;

    // Getting a cursor
    handle->dbHandle->cursor(handle->dbHandle, NULL, &cursorp, 0);

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
            // De-serialize the pagedata
            SerializePage ser((char*)data.data, data.size);
            // Create the PageData
            PageData page( ser.getUrlId(), ser.getFragId(), ser.getXpath() );
            // Creating an answer

            answer = new MwsAnsw(page.getFullUrl( &(handle->url_dict) ).c_str(),
                                 page.getXpath());
            // Computing the substitutions
            answer->subst.qvarXpaths.reserve(resultAnswset->qvarXpaths.size());
            for (it  = resultAnswset->qvarXpaths.begin();
                 it != resultAnswset->qvarXpaths.end();
                 it ++)
            {
                answer->subst.qvarXpaths.push_back(page.getXpath() + *it);
            }
            // Adding the answer
            resultAnswset->answers.push_back(answer);

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

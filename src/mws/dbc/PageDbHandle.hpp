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
 * Header of PageDbHandle class
 *
 * Date: 12.III.2013
 * Author: Daniel Hasegan d.hasegan@jacobs-university.de
 */
#ifndef _PAGEDBHANDLE_HPP
#define _PAGEDBHANDLE_HPP

// System includes
#include <db.h>
#include <pthread.h>
#include <stdint.h>

// Local includes
#include "mws/dbc/PageDbConn.hpp"
#include "mws/types/URLDictionary.hpp"
#include "mws/types/MwsAnswset.hpp"

class PageDbConn;

class PageDbHandle {
private:

    std::string dbenvDir;
    /// Database environment handle
    DB_ENV* dbEnv;
    /// Database handle
    DB*     dbHandle;
    /// Mutual exclusion lock for the class
    pthread_mutex_t lock;
    /// Condition to signal on closing instances
    pthread_cond_t  instanceClosed;
    /// Count on number of alive instances
    unsigned int alive;
    /// Url Dictionary
    mws::URLDictionary url_dict;

public:

    /// Constructor of the database
    PageDbHandle();

    /// Initialization of the Page Database
    int init(std::string dbenv_dir);

    /// Cleanup of the Page Database
    void clean();

    /// Create a connection that will insert or query the Db
    PageDbConn* createConnection();

    /// Friend declarations
    friend class PageDbConn;
};


#endif

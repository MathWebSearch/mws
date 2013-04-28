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
#ifndef _MWSDBCONN_HPP
#define _MWSDBCONN_HPP

/**
  * @brief File containing the header of the MwsDatabase Connection class.
  * @file MwsDbConn.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date 17 May 2011
  *
  * License: GPL v3
  *
  */

// System includes

#include <db.h>
#include <pthread.h>
#include <string>

// Local includes

#include "mws/types/MwsAnswset.hpp"


namespace mws
{

/**
  * @brief Mws database connection class
  */
class MwsDbConn
{
private:
    static std::string dbenvDir;
    /// Database environment handle
    static DB_ENV* dbEnv;
    /// Database handle
    static DB*     dbHandle;
    /// Mutual exclusion lock for the class
    static pthread_mutex_t lock;
    /// Condition to signal on closing instances
    static pthread_cond_t  instanceClosed;
    /// Count on number of alive instances
    static unsigned int alive;

public:
    /**
      * Default constructor of the MwsDbConn class.
      */
    MwsDbConn();

    /**
      * Default destructor of the MwsDbConn class.
      */
    ~MwsDbConn();

    /**
      * @brief Method to initialize the MWS Database module.
      * @return 0 on success and non-zero on failure.
      */
    static int init(std::string dbenv_dir);

    /**
      * @brief Method to clean up the MWS Database module.
      */
    static void clean();

    /**
      * @brief Method to insert a record in the database.
      * @param url_uuid is the url and uuid of the m:math where the record was
      * found.
      * @param xpath is the xpath within the m:math where the record is
      * @param id is the identifier of its place in the index.
      * @return 0 on success and non-zero on failure.
      */
    int insert(const char*        url_uuid,
               const char*        xpath,
               unsigned long long id);

    /**
      * @brief Method to query the addresses for an id from the MWS database.
      * @param id is the node id to be queried.
      * @param limitMin is the index where to start returning.
      * @param limitSize is the maximum number of results to be returned.
      * MathWebSearch Answer set.
      * @param resultAnswset is a pointer to the MWS Answer set where the
      * results are added.
      * @return the number of matching queries added to the resultAnswset or -1
      in case of database error.
      */
    int query(unsigned long long id,
              unsigned int       limitMin,
              unsigned int       limitSize,
              mws::MwsAnswset*   resultAnswset);
};

}

#endif

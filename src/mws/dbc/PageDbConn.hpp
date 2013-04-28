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
 * Header of PageDbConn class
 *
 * Date: 12.III.2013
 * Author: Daniel Hasegan d.hasegan@jacobs-university.de
 */
#ifndef _PAGEDBCONN_HPP
#define _PAGEDBCONN_HPP

// System includes

// Local includes
#include "mws/dbc/PageDbHandle.hpp"
#include "mws/types/PageData.hpp"
#include "mws/types/MwsAnswset.hpp"

class PageDbHandle;

class PageDbConn {
private:
    PageDbHandle* handle;

    /**
     * @brief Connection constructor that can only be accesed by the PageDbHandle
     */
    PageDbConn(PageDbHandle* aHandle);
public:

    /**
     * @brief Connection Destructor
     */
    ~PageDbConn();

    /**
     * @brief insert pageData into the Database
     */
    int insert(const char* url_uuid,const char* xpath,unsigned long long id);

    /**
     * @brief query the Db for the id and return the results in pageData pointer
     */
    int query(unsigned long long id,
              unsigned int limitMin,
              unsigned int limitSize,
              mws::MwsAnswset* resultAnswset);

    /// Friend declarations
    friend class PageDbHandle;

private:

    /// Unused
    PageDbConn();
};


#endif

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
#ifndef _URLDB_HPP
#define _URLDB_HPP

/*
 * Class for Crawled URLs Database
 * author: Catalin Perticas
 * date: 17 Aug 2012
 */

// System includes

#include <string>
#include <db.h>

// Local includes

namespace mws
{

class urlDB
{
private:
    // Database handle
    DB *urldb;
    // Key is url and Data is last_modified

public:
    // Database constructor
    urlDB();

    // Database destructor
    ~urlDB();

    // opens the Database of crawled URLs and creates the handle for it
    void start_DB();

    // checks for the existence of an URL in the Database
    bool is_url_in_DB(std::string url);

    // adds a new url to the Database
    void add_url_to_DB(std::string url, std::string last_modified);

    // set last_modified to the latest version(remove old version of URL from Database + add new version of URL to Database)
    void update_url_in_DB(std::string url, std::string last_modified);

    // removes URL from the Database
    void remove_url_from_DB(std::string url);

    // checks whether an URL has to be recrawled(if the latest version is different from the one we crawled)
    bool have_to_recrawl_url(std::string url, std::string last_modified);

    // closes the Database of crawled URLs
    void end_DB();

};

}
#endif // _URLDB_HPP


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
/*
 * Implementation of the Class for Crawled URLs Database
 * author: Catalin Perticas
 * date: 17 Aug 2012
 */

//System includes
#include <cstring>
#include <cstdio> 
#include <cstdlib>
#include <iostream>
#include <sys/types.h>

// Local includes

#include "UrlDB.hpp"
#include "crawler/types/Page.hpp"

using namespace std;
using namespace mws;

#define	DATABASE "urldb.db"

// Implementation of Database constructor
urlDB::urlDB()
{
    start_DB();
}

// Implementation of Database destructor
urlDB::~urlDB()
{
    end_DB();
}

// Implementation of Database start: opens the Database of crawled URLs and creates the handle for it
void urlDB::start_DB() 
{
    cout << "Opening the database" << endl;
    int ret = db_create(&urldb, NULL, 0);
    if (ret != 0)
    {
        fprintf(stderr, "db_create: %s\n", db_strerror(ret));
        exit (ret);
    }
    ret = urldb->open(urldb, NULL, DATABASE, NULL, DB_BTREE, DB_CREATE, 0664);
    if (ret != 0)
    {
        urldb->err(urldb, ret, "%s", DATABASE);
        end_DB();
        exit(ret);
    }
}

// Implementation of the function that checks for the existence of an URL in the Database
bool urlDB::is_url_in_DB(string url) 
{
    DBT key;
    memset(&key, 0, sizeof(key));
    key.data = (void*)url.c_str();
    key.size = sizeof(url.c_str());
    //cout << "Checking whether " << url << " is in the database" << endl;
    int ret = urldb->exists(urldb, NULL, &key, 0);
    if (ret == DB_NOTFOUND) return false;
    else return true;
} 

// Implementation of the function that adds URLs in the Database
void urlDB::add_url_to_DB(string url, string last_modified) 
{
    DBT key, data;
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.data = (void*)url.c_str();
    key.size = sizeof(url.c_str());
    data.data = (void*)last_modified.c_str();
    data.size = sizeof(last_modified.c_str());

    //cout << "Adding " << url << endl;
    int ret = urldb->put(urldb, NULL, &key, &data, 0);
    if (ret != 0)
    {
        urldb->err(urldb, ret, "DB->put");
        end_DB();
        exit(ret);
    }
}

// Implemenatation of the function that removes URLs from the Database
void urlDB::remove_url_from_DB(string url)
{
    DBT key;
    memset(&key, 0, sizeof(key));
    key.data = (void*)url.c_str();
    key.size = sizeof(url.c_str());

    //cout << "Removing " << url << endl;
    int ret = urldb->del(urldb, NULL, &key, 0);
    if (ret != 0)
    {
        urldb->err(urldb, ret, "DB->del");
        end_DB();
        exit(ret);
    }
}

// Implementation of the function that sets last_modified to the latest version
// (removes old version of URL from Database + adds new version of URL to Database)
void urlDB::update_url_in_DB(string url, string last_modified)
{
    //cout << "Updating " << url << " to version from " << last_modified << endl;
    remove_url_from_DB(url);
    add_url_to_DB(url, last_modified);
}

// Implementation of the function that checks whether we have to recrawl an URL
bool urlDB::have_to_recrawl_url(string url, string last_modified)
{
    DBT key, data;
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    key.data = (void*)url.c_str();
    key.size = sizeof(url.c_str());

    //cout << "Fetching " << url << endl;
    int ret = urldb->get(urldb, NULL, &key, &data, 0);
    if (ret != 0)
    {
        urldb->err(urldb, ret, "DB->get");
        end_DB();
        exit(ret);
    }

    string cur_last_modified((char*)data.data);
    if (last_modified != cur_last_modified) return true;
    return false;
}

// Implementation of Database end: closes the Database of crawled URLs
void urlDB::end_DB()
{
    cout << "Closing the database" << endl;
    int ret;
    ret = urldb->close(urldb, 0);
    if (ret != 0) exit(ret);
}

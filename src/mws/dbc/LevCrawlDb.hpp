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

#ifndef _MWS_DBC_LEVCRAWLDB_HPP
#define _MWS_DBC_LEVCRAWLDB_HPP

/**
  * @file LevCrawlDb.hpp
  * @brief LevelDb Crawl Database API
  * @author Radu Hambasan
  * @date 11 Dec 2013
  */


#include <stdexcept>
#include <string>
#include <leveldb/db.h>

#include "mws/dbc/CrawlDb.hpp"

namespace mws {
namespace dbc {

class LevCrawlDb : public CrawlDb {
 public:
    LevCrawlDb();
    virtual ~LevCrawlDb();

    void open(const char* path) throw (std::runtime_error);
    void create_new(const char* path, bool deleteIfExists)
    throw (std::runtime_error);

    /**
     * @brief insert crawled data
     * @param crawlId id of the crawl element
     * @param crawlData data associated with the crawl element
     *
     */
    virtual CrawlId putData(const CrawlData& crawlData)
    throw (std::exception);

    /**
     * @brief get crawled data
     * @param crawlId id of the crawl element
     * @return CrawlData corresponding to crawlId
     * @throw NotFound or I/O exceptions
     */
    virtual const CrawlData getData(const CrawlId& crawlId)
    throw (std::exception);

 private:
    leveldb::DB* mDatabase;
    CrawlId mNextCrawlId;
};

}  // namespace dbc
}  // namespace mws

#endif  // _MWS_DBC_MEMCRAWLDB_HPP

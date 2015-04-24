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
#ifndef _MWS_DBC_CRAWLDB_HPP
#define _MWS_DBC_CRAWLDB_HPP

/**
  * @file CrawlDb.hpp
  * @brief Crawl Database interface
  * @date 12 Nov 2013
  */

#include <string>

namespace mws {
namespace dbc {

typedef uint32_t CrawlId;
const CrawlId CRAWLID_NULL = 0;

typedef std::string CrawlData;
const CrawlData CRAWLDATA_NULL = CrawlData();

class CrawlDb {
public:
    virtual ~CrawlDb() {}

    /**
     * @brief insert crawled data
     * @param crawlId id of the crawl element
     * @param crawlData data associated with the crawl element
     * @throw exception
     */
    virtual CrawlId putData(const CrawlData& crawlData) = 0;

    /**
     * @brief get crawled data
     * @param crawlId id of the crawl element
     * @return CrawlData corresponding to crawlId
     * @throw NotFound or I/O exceptions
     */
    virtual const CrawlData getData(const CrawlId& crawlId) = 0;
};

}  // namespace dbc
}  // namespace mws

#endif  // _MWS_DBC_CRAWLDB_HPP

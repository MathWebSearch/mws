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
#ifndef _MWS_DBC_MEMCRAWLDB_HPP
#define _MWS_DBC_MEMCRAWLDB_HPP

/**
  * @file CrawlDb.hpp
  * @brief Crawl Data Memory Database declarations
  * @date 12 Nov 2013
  */

#include <map>

#include "mws/types/NodeInfo.hpp"

#include "CrawlDb.hpp"

namespace mws { namespace dbc {

class MemCrawlDb : public CrawlDb {
public:
    MemCrawlDb();

    /**
     * @brief insert crawled data
     * @param crawlId id of the crawl element
     * @param crawlData data associated with the crawl element
     *
     */
    virtual mws::CrawlId putData(const mws::CrawlData& crawlData)
    throw (std::exception);

    /**
     * @brief get crawled data
     * @param crawlId id of the crawl element
     * @return CrawlData corresponding to crawlId
     * @throw NotFound or I/O exceptions
     */
    virtual const mws::CrawlData* getData(const mws::CrawlId& crawlId)
    throw (std::exception);

private:
    std::map<mws::CrawlId, mws::CrawlData> mData;
    CrawlId mNextCrawlId;
};

} }

#endif // _MWS_DBC_MEMCRAWLDB_HPP

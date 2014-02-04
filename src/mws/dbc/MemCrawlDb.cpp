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
  * @file MemCrawlDb.cpp
  * @brief Crawl Data Memory Database implementation
  * @date 12 Nov 2013
  */

#include <map>
#include <stdexcept>
#include "common/utils/ToString.hpp"

#include "MemCrawlDb.hpp"

using namespace std;

namespace mws { namespace dbc {

MemCrawlDb::MemCrawlDb() : mNextCrawlId(0) {

}

mws::CrawlId
MemCrawlDb::putData(const mws::types::CrawlData& crawlData)
throw (std::exception) {
    const CrawlId crawlId = mNextCrawlId++;
    auto ret = mData.insert(make_pair(crawlId, crawlData));
    if (!ret.second) {
        throw std::runtime_error("Duplicate entry at crawlId = " +
                                 ToString(crawlId));
    }

    return crawlId;
}

const types::CrawlData MemCrawlDb::getData(const mws::CrawlId& crawlId)
throw (std::exception) {
    auto it = mData.find(crawlId);
    if (it != mData.end()) {
        return it->second;
    } else {
        throw runtime_error("No data corresponding to crawlId = " +
                            ToString(crawlId));
    }
}


} }

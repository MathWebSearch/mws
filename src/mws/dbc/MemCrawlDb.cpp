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
  * @file CrawlDb.cpp
  * @brief Crawl Data Memory Database implementation
  * @date 12 Nov 2013
  */

#include "MemCrawlDb.hpp"

#include <map>

using namespace std;

namespace mws { namespace dbc {

int
MemCrawlDb::putData(const mws::CrawlId&     crawlId,
                    const mws::CrawlData&   crawlData) {
    auto ret = mData.insert(make_pair(crawlId, crawlData));
    if (ret.second) {
        return 0;
    } else {
        return -1;
    }
}

const mws::CrawlData*
MemCrawlDb::getData(const mws::CrawlId&   crawlId) {
    auto it = mData.find(crawlId);
    if (it != mData.end()) {
        return &(it->second);
    } else {
        return NULL;
    }
}


} }

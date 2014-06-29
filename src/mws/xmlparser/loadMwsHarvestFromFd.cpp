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
  * @brief File containing the implementation of the loadMwsHarvestFromFd
  *
  * @file loadMwsHarvestFromFd.cpp
  * @author Radu Hambasan
  * @date 30 Apr 2014
  *
  * License: GPL v3
  *
  */

#include <string.h>
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <utility>
using std::pair;

#include "mws/dbc/CrawlDb.hpp"
using mws::dbc::CrawlId;
#include "mws/index/IndexBuilder.hpp"
using mws::index::IndexingOptions;
#include "mws/types/CmmlToken.hpp"
using mws::types::CmmlToken;
#include "common/utils/compiler_defs.h"

#include "mws/xmlparser/processMwsHarvest.hpp"

using mws::index::IndexBuilder;

namespace mws {
namespace parser {

class HarvestIndexer : public HarvestProcessor {
 public:
    int processExpression(const CmmlToken* tok, const string& exprUri,
                          const uint32_t& crawlId);
    CrawlId processData(const string& data);
    explicit HarvestIndexer(IndexBuilder* indexBuilder);

 private:
    IndexBuilder* indexBuilder;
};

HarvestIndexer::HarvestIndexer(index::IndexBuilder* indexBuilder)
    : indexBuilder(indexBuilder) {}

int HarvestIndexer::processExpression(const CmmlToken* token,
                                      const string& exprUri,
                                      const uint32_t& crawlId) {
    return indexBuilder->indexContentMath(token, exprUri, crawlId);
}

CrawlId HarvestIndexer::processData(const string& data) {
    return indexBuilder->indexCrawlData(data);
}

/**
 * @brief loadMwsHarvestFromFd build an index with data from fd
 * @param indexBuilder
 * @param fd file descriptor of the harvest file
 * @return pair, where the first item is the return code and
 * the second is the number of parsed expressions
 * XXX: does NOT close the file descriptor
 */
pair<int, int> loadMwsHarvestFromFd(mws::index::IndexBuilder* indexBuilder,
                                    int fd) {
    HarvestProcessor* harvestIndexer = new HarvestIndexer(indexBuilder);

    auto ret = processMwsHarvest(fd, harvestIndexer);
    delete harvestIndexer;
    return ret;
}

}  // namespace parser
}  // namespace mws

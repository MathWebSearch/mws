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
#include <map>
using std::map;

#include "mws/dbc/CrawlDb.hpp"
using mws::dbc::CrawlId;
#include "mws/index/IndexManager.hpp"
using mws::index::IndexingOptions;
#include "mws/types/CmmlToken.hpp"
#include "common/utils/compiler_defs.h"

#include "processMwsHarvest.hpp"

using namespace std;
using namespace mws::types;
using namespace mws::dbc;
using mws::index::IndexManager;

namespace mws {
namespace parser {

class HarvestIndexer : public HarvestProcessor {
 public:
    int processExpression(const CmmlToken *tok,
                          const string &exprUri, const uint32_t &crawlId);
    CrawlId processData(const string &data);
    explicit HarvestIndexer(IndexManager* indexManager);
 private:
    IndexManager* indexManager;
};

HarvestIndexer::HarvestIndexer(index::IndexManager *indexManager) :
    indexManager(indexManager) {
}

int HarvestIndexer::processExpression(const CmmlToken *tok,
                                      const string &exprUri,
                                      const uint32_t &crawlId) {
    if (crawlId == CRAWLID_NULL) {
        return indexManager->indexContentMath(tok, exprUri);
    } else {
        return indexManager->indexContentMath(tok, exprUri, crawlId);
    }
}

CrawlId HarvestIndexer::processData(const string &data) {
    return indexManager->indexCrawlData(data);
}


pair<int,int>
loadMwsHarvestFromFd(mws::index::IndexManager *indexManager, int fd) {
    HarvestProcessor* harvestIndexer = new HarvestIndexer(indexManager);

    auto ret = processMwsHarvest(fd, harvestIndexer);
    delete harvestIndexer;
    return ret;
}

}  // namespace parser
}  // namespace mws

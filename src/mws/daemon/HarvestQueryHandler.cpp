/*

Copyright (C) 2010-2014 KWARC Group <kwarc.info>

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
  * @brief HarvestQueryHandler implementation
  * @file HarvestQueryHandler.cpp
  * @author Corneliu-Claudiu Prodescu
  * @date 18 Jun 2011
  *
  * @edited Daniel Hasegan
  * @date 13 Aug 2012
  *
  * @edited Radu Hambasan
  * @date 13 Dec 2013
  *
  * License: GPL v3
  */

#include <cinttypes>
#include <vector>
using std::vector;

#include "common/utils/compiler_defs.h"
#include "mws/index/encoded_token.h"
#include "mws/types/Query.hpp"
using mws::types::Query;
#include "mws/index/ExpressionEncoder.hpp"
using mws::index::QueryEncoder;
using mws::index::ExpressionEncoder;
using mws::index::ExpressionInfo;
#include "mws/index/TmpIndexAccessor.hpp"
using mws::index::TmpIndexAccessor;
#include "mws/index/IndexBuilder.hpp"
using mws::index::IndexBuilder;
using mws::index::HarvesterConfiguration;
using mws::index::loadHarvests;
#include "mws/query/SearchContext.hpp"
using mws::query::SearchContext;
#include "mws/daemon/HarvestQueryHandler.hpp"

using mws::index::loadHarvests;

namespace mws {
namespace daemon {

HarvestQueryHandler::HarvestQueryHandler(const HarvesterConfiguration& config)
    : _encodingConfig(config.encoding) {
    IndexBuilder indexBuilder(&_formulaDb, &_crawlDb, &_index,
                              &_meaningDictionary, config.encoding);
    uint64_t numExpressions = loadHarvests(&indexBuilder, config);
    PRINT_LOG("%" PRIu64 " expressions loaded.\n", numExpressions);
}

HarvestQueryHandler::~HarvestQueryHandler() {}

MwsAnswset* HarvestQueryHandler::handleQuery(Query* mwsQuery) {
    MwsAnswset* result;

    QueryEncoder encoder(&_meaningDictionary);
    vector<encoded_token_t> encodedQuery;
    ExpressionInfo queryInfo;

    if (encoder.encode(_encodingConfig, mwsQuery->tokens[0],
                       &encodedQuery, &queryInfo) == 0) {
        dbc::DbQueryManager dbQueryManger(&_crawlDb, &_formulaDb);
        SearchContext ctxt(encodedQuery, mwsQuery->options);
        result = ctxt.getResult<TmpIndexAccessor>(
            &_index, &dbQueryManger, mwsQuery->attrResultLimitMin,
            mwsQuery->attrResultMaxSize, mwsQuery->attrResultTotalReqNr);
    } else {
        result = new MwsAnswset();
    }

    result->qvarNames = queryInfo.qvarNames;
    result->qvarXpaths = queryInfo.qvarXpaths;

    return result;
}

}  // namespace daemon
}  // namespace mws

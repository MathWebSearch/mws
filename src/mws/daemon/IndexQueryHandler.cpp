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
  * @brief File containing the implementation of the IndexDaemon class.
  * @file IndexDaemon.cpp
  * @author Corneliu-Claudiu Prodescu
  * @date 18 Jun 2011
  *
  * @edited Radu Hambasan
  * @date 18 Feb 2014
  *
  * License: GPL v3
  */

// System includes

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>  // Primitive System datatypes
#include <sys/stat.h>   // POSIX File characteristics
#include <fcntl.h>      // File control operations
#include <stdlib.h>

#include <string>
using std::string;
#include <vector>
using std::vector;
#include <fstream>
using std::filebuf;
using std::istream;
using std::ios;
#include <stdexcept>
using std::exception;
#include <memory>
using std::unique_ptr;

#include "mws/dbc/CrawlDb.hpp"
using mws::dbc::CrawlData;
#include "mws/dbc/LevFormulaDb.hpp"
using mws::dbc::LevFormulaDb;
#include "mws/dbc/LevCrawlDb.hpp"
using mws::dbc::LevCrawlDb;
#include "mws/dbc/DbQueryManager.hpp"
using mws::dbc::DbQueryManager;
using mws::dbc::DbAnswerCallback;
#include "mws/index/index.h"
#include "mws/index/IndexLoader.hpp"
using mws::index::IndexLoader;
#include "mws/index/ExpressionEncoder.hpp"
using mws::index::QueryEncoder;
using mws::index::ExpressionInfo;
#include "mws/index/MeaningDictionary.hpp"
using mws::index::MeaningDictionary;
#include "mws/index/IndexAccessor.hpp"
using mws::index::IndexAccessor;
#include "mws/query/SearchContext.hpp"
using mws::query::SearchContext;
#include "mws/query/engine.h"
#include "mws/types/FormulaPath.hpp"
using mws::types::FormulaId;
using mws::types::FormulaPath;
#include "mws/types/Query.hpp"
using mws::types::Query;
#include "mws/xmlparser/processMwsHarvest.hpp"
#include "mws/daemon/IndexQueryHandler.hpp"

namespace mws {
namespace daemon {

struct HandlerStruct {
    MwsAnswset* result;
    types::Query* mwsQuery;
    DbQueryManager* dbQueryManager;
};

static result_cb_return_t result_callback(void* _ctxt, const leaf_t* leaf) {
    assert(leaf->type == LEAF_NODE);

    HandlerStruct* ctxt = reinterpret_cast<HandlerStruct*>(_ctxt);
    MwsAnswset* result = ctxt->result;
    Query* mwsQuery = ctxt->mwsQuery;
    DbQueryManager* dbQueryManager = ctxt->dbQueryManager;
    DbAnswerCallback queryCallback = [result](const FormulaPath& formulaPath,
                                              const CrawlData& crawlData) {
        auto answer = new mws::types::Answer();
        answer->data = crawlData;
        answer->uri = formulaPath.xmlId;
        answer->xpath = formulaPath.xpath;
        result->answers.push_back(answer);
        return 0;
    };

    dbQueryManager->query((FormulaId)leaf->formula_id,
                          mwsQuery->attrResultLimitMin,
                          mwsQuery->attrResultMaxSize, queryCallback);
    result->total += leaf->num_hits;

    return QUERY_CONTINUE;
}

MwsAnswset* IndexQueryHandler::handleQuery(Query* query) {
    MwsAnswset* result;
    QueryEncoder encoder(_index.getMeaningDictionary());
    vector<encoded_token_t> encodedQuery;
    ExpressionInfo queryInfo;

    if (encoder.encode(_config.encoding, query->tokens[0], &encodedQuery,
                       &queryInfo) == 0) {
        if (_config.useExperimentalQueryEngine) {
            HandlerStruct ctxt;
            ctxt.result = result = new MwsAnswset();
            ctxt.mwsQuery = query;
            ctxt.dbQueryManager = _index.getDbQueryManager();

            encoded_formula_t encodedFormula;
            encodedFormula.data = encodedQuery.data(),
            encodedFormula.size = encodedQuery.size();

            query_engine_run(_index.getIndexHandle(), &encodedFormula,
                             result_callback, &ctxt);
        } else {
            SearchContext ctxt(encodedQuery, query->options);
            result = ctxt.getResult<IndexAccessor>(
                _index.getIndexHandle(), _index.getDbQueryManager(),
                query->attrResultLimitMin, query->attrResultMaxSize,
                query->attrResultTotalReqNr);
        }
    } else {
        result = new MwsAnswset();
    }

    result->qvarNames = queryInfo.qvarNames;
    result->qvarXpaths = queryInfo.qvarXpaths;

    return result;
}

IndexQueryHandler::IndexQueryHandler(const std::string& indexPath,
                                     const Config& config)
    : _index(indexPath), _config(config) {}

IndexQueryHandler::~IndexQueryHandler() {}

}  // namespace daemon
}  // namespace mws

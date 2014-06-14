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
  * @brief given a compressed index and the original harvest, this function
  * generates JSON content for Elastic search
  *
  * @file loadMwsHarvestFromFd.cpp
  * @author Radu Hambasan
  * @date 04 May 2014
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
#include <stack>
using std::stack;
#include <map>
using std::map;

#include "mws/dbc/FormulaDb.hpp"
using namespace mws::dbc;
using mws::types::FormulaId;
#include "mws/index/IndexBuilder.hpp"
using mws::index::IndexingOptions;
#include "mws/types/CmmlToken.hpp"
using mws::types::CmmlToken;
using mws::types::TokenCallback;
#include "mws/types/Query.hpp"
using mws::types::Query;
#include "common/utils/compiler_defs.h"
#include "mws/index/IndexAccessor.hpp"
using mws::index::IndexAccessor;
#include "mws/query/SearchContext.hpp"
using mws::query::SearchContext;
#include "mws/index/ExpressionEncoder.hpp"
#include "mws/daemon/Daemon.hpp"
#include "mws/index/MeaningDictionary.hpp"
using mws::index::MeaningDictionary;
using mws::daemon::Config;
using mws::types::FormulaPath;
#include "mws/xmlparser/processMwsHarvest.hpp"

namespace mws {
namespace parser {

class HarvestParser : public HarvestProcessor {
 public:
    int processExpression(const CmmlToken* tok, const string& exprUri,
                          const uint32_t& crawlId);
    CrawlId processData(const string& data);
    vector<ParseResult*> getParseResults();
    HarvestParser(IndexAccessor::Index* index,
                  MeaningDictionary* meaningictionary, Config config);

 private:
    IndexAccessor::Index* _index;
    MeaningDictionary* _meaningDictionary;
    Config _config;
    CrawlId _idCounter;
    Query::Options _queryOptions;
    /* each CrawlId uniquely identifies a document */
    map<CrawlId, ParseResult*> documents;
};

HarvestParser::HarvestParser(IndexAccessor::Index* index,
                             MeaningDictionary* meaningDictionary,
                             Config config)
    : _index(index),
      _meaningDictionary(meaningDictionary),
      _config(std::move(config)),
      _idCounter(0) {
    _queryOptions.includeHits = false;
}

vector<ParseResult*> HarvestParser::getParseResults() {
    vector<ParseResult*> results;
    results.reserve(documents.size());

    for (auto& kv : documents) {
        results.push_back(kv.second);
    }

    return results;
}

int HarvestParser::processExpression(const CmmlToken* token,
                                     const string& exprUri,
                                     const uint32_t& crawlId) {
    assert(token != nullptr);
    assert(crawlId <= _idCounter);

    index::HarvestEncoder encoder(_meaningDictionary);
    u_int32_t numSubExpressions = 0;

    TokenCallback tokCallback = [&](const CmmlToken* tok) {
        vector<encoded_token_t> encodedFormula;
        encoder.encode(_config.indexingOptions, tok, &encodedFormula, nullptr);

        MwsAnswset* result;
        SearchContext ctxt(encodedFormula, _queryOptions);
        result = ctxt.getResult<IndexAccessor>(_index,
                                               /* dbQueryManager = */ nullptr,
                                               /* offset = */ 0,
                                               /* size = */ 1,
                                               /* maxTotal = */ 1);
        numSubExpressions += result->total;
        auto hit = new Hit();
        FormulaId fmId = *(result->ids.begin());
        hit->uri = exprUri;
        hit->xpath = tok->getXpath();

        ParseResult* doc = documents[crawlId];
        (doc->idMappings[fmId]).push_back(hit);
        delete result;
    };

    token->foreachSubexpression(tokCallback);

    return numSubExpressions;
}

CrawlId HarvestParser::processData(const string& data) {
    documents[++_idCounter] = new ParseResult();
    documents[_idCounter]->data = data;

    return _idCounter;
}

vector<ParseResult*> parseMwsHarvestFromFd(const Config& config,
                                           IndexAccessor::Index* index,
                                           MeaningDictionary* meaningDictionary,
                                           int fd) {
    HarvestParser harvestParser(index, meaningDictionary, config);

    processMwsHarvest(fd, &harvestParser);

    return harvestParser.getParseResults();
}

}  // namespace parser
}  // namespace mws

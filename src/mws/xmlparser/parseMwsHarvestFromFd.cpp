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

#include "mws/dbc/FormulaDb.hpp"
using namespace mws::dbc;
using mws::types::FormulaId;
#include "mws/index/IndexManager.hpp"
using mws::index::IndexingOptions;
#include "mws/types/CmmlToken.hpp"
using mws::types::CmmlToken;
using mws::types::TokenCallback;
#include "common/utils/compiler_defs.h"
#include "mws/index/IndexAccessor.hpp"
using mws::index::IndexAccessor;
#include "mws/query/SearchContext.hpp"
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
    int processExpression(const CmmlToken *tok,
                          const string &exprUri, const uint32_t &crawlId);
    CrawlId processData(const string &data);
    string getParsedData() { return _parsedData; }
    vector<Hit*> getIdMappings() { return _idMappings; }
    HarvestParser(IndexAccessor::Index* index,
                  MeaningDictionary* meaningictionary,
                  const Config& config, FormulaDb* formulaDb);
 private:
    string _parsedData;
    IndexAccessor::Index* _index;
    MeaningDictionary* _meaningDictionary;
    FormulaDb* _formulaDb;
    Config _config;
    vector<Hit*> _idMappings;
};

HarvestParser::HarvestParser(IndexAccessor::Index* index,
                             MeaningDictionary* meaningDictionary,
                             const Config& config, FormulaDb* formulaDb) :
    _parsedData(""), _index(index), _meaningDictionary(meaningDictionary),
    _formulaDb(formulaDb), _config(config) {
}

int HarvestParser::processExpression(const CmmlToken* token,
                                      const string& exprUri,
                                      const uint32_t& crawlId) {
    UNUSED(crawlId);
    assert(token != NULL);
    MwsAnswset* result;
    query::SearchContext* ctxt;
    dbc::DbQueryManager dbQueryManager(NULL, _formulaDb);
    index::HarvestEncoder encoder(_meaningDictionary);
    u_int32_t numSubExpressions = 0;

    TokenCallback tokCallback = [&] (const CmmlToken* tok) {
        vector<encoded_token_t> encodedFormula;
        encoder.encode(_config.indexingOptions, tok,
                       &encodedFormula, NULL);
        ctxt = new query::SearchContext(encodedFormula);
        result = ctxt->getResult<IndexAccessor>(_index, &dbQueryManager,
                                                /* offset=*/ 0,
                                                /* size=*/ 1,
                                                /* maxTotal=*/ 1);
        numSubExpressions += result->total;
        Hit* hit = new Hit;
        hit->id = *(result->ids.begin());
        hit->uri = exprUri;
        hit->xpath = tok->getXpath();
        _idMappings.push_back(hit);
        delete result;
        delete ctxt;
    };

    token->foreachSubexpression(tokCallback);

    return numSubExpressions;
}

CrawlId HarvestParser::processData(const string& data) {
    this->_parsedData = data;
    return CRAWLID_NULL;  // we do not want to put this into a database
}

ParseResult
parseMwsHarvestFromFd(const Config& config, IndexAccessor::Index* index,
                      MeaningDictionary* meaningDictionary,
                      FormulaDb* formulaDb, int fd) {
    HarvestProcessor* harvestParser = new HarvestParser(index,
                                                        meaningDictionary,
                                                        config, formulaDb);

    processMwsHarvest(fd, harvestParser);

    HarvestParser* hvp = (HarvestParser*) harvestParser;
    ParseResult ret = ParseResult(hvp->getParsedData(), hvp->getIdMappings());
    delete harvestParser;
    return ret;
}

}  // namespace parser
}  // namespace mws

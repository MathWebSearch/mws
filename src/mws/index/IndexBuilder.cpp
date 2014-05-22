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
  * @file IndexBuilder.cpp
  * @brief Indexing Manager implementation
  * @date 18 Nov 2013
  */

#include <assert.h>

#include <set>
using std::set;
#include <stack>
using std::stack;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <unordered_map>
using std::unordered_map;

#include "mws/types/CmmlToken.hpp"
using mws::types::CmmlToken;
using mws::types::TokenCallback;
#include "mws/types/FormulaPath.hpp"
using mws::types::FormulaId;
using mws::types::FormulaPath;
#include "mws/dbc/CrawlDb.hpp"
using mws::dbc::CrawlId;
using mws::dbc::CrawlData;
#include "mws/index/ExpressionEncoder.hpp"
#include "mws/index/IndexBuilder.hpp"

namespace mws { namespace index {

IndexBuilder::IndexBuilder(dbc::FormulaDb* formulaDb,
                           dbc::CrawlDb* crawlDb,
                           TmpIndex* index,
                           MeaningDictionary* meaningDictionary,
                           const IndexingOptions& indexingOptions) :
    m_formulaDb(formulaDb), m_crawlDb(crawlDb), m_index(index),
    m_meaningDictionary(meaningDictionary),
    m_indexingOptions(indexingOptions) {
}

CrawlId
IndexBuilder::indexCrawlData(const CrawlData& crawlData) {
    return m_crawlDb->putData(crawlData);
}

int
IndexBuilder::indexContentMath(const CmmlToken* cmmlToken,
                               const string xmlId,
                               const CrawlId& crawlId) {
    assert(cmmlToken != NULL);
    set<FormulaId> uniqueFormulaIds;
    HarvestEncoder encoder(m_meaningDictionary);
    int numSubExpressions = 0;

    cmmlToken->foreachSubexpression([&](const CmmlToken* token) {
        vector<encoded_token_t> encodedFormula;
        encoder.encode(m_indexingOptions, token,
                       &encodedFormula, NULL);
        TmpLeafNode* leaf = m_index->insertData(encodedFormula);
        FormulaId formulaId = leaf->id;
        auto ret = uniqueFormulaIds.insert(formulaId);
        if (ret.second) {
            types::FormulaPath formulaPath;
            formulaPath.xmlId = xmlId;
            formulaPath.xpath = token->getXpath();
            m_formulaDb->insertFormula(leaf->id, crawlId, formulaPath);
            leaf->solutions++;
            numSubExpressions++;
        }
    });

    return numSubExpressions;
}

} }


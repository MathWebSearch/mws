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
  * @file IndexManager.cpp
  * @brief Indexing Manager implementation
  * @date 18 Nov 2013
  */

#include <set>
using std::set;
#include <stack>
using std::stack;
#include <string>
using std::string;

#include "mws/types/CmmlToken.hpp"
using mws::types::CmmlToken;
#include "mws/types/NodeInfo.hpp"
using mws::types::CrawlId;
using mws::types::CrawlData;
using mws::types::FormulaId;
using mws::types::FormulaPath;

#include "IndexManager.hpp"

namespace mws { namespace index {

IndexManager::IndexManager(dbc::FormulaDb* formulaDb,
                           dbc::CrawlDb* crawlDb,
                           MwsIndexNode* index,
                           types::MeaningDictionary* meaningDictionary) :
    m_formulaDb(formulaDb), m_crawlDb(crawlDb), m_index(index),
    m_meaningDictionary(meaningDictionary) { }

types::CrawlId
IndexManager::indexCrawlData(const types::CrawlData& crawlData) {
    return m_crawlDb->putData(crawlData);
}

int
IndexManager::indexContentMath(const types::CmmlToken* cmmlToken,
                               const std::string xmlId,
                               const types::CrawlId& crawlId) {
    assert(cmmlToken != NULL);
    // Using a stack to insert all subterms by
    // going depth first through the CmmlToken
    set<FormulaId> uniqueFormulaIds;
    stack<const CmmlToken*> subtermStack;
    int numSubExpressions = 0;

    subtermStack.push(cmmlToken);
    while (!subtermStack.empty()) {
        const CmmlToken* currentSubterm = subtermStack.top();
        subtermStack.pop();

        for (auto rIt  = currentSubterm->getChildNodes().rbegin();
             rIt != currentSubterm->getChildNodes().rend();
             rIt ++) {
            subtermStack.push(*rIt);
        }

        MwsIndexNode* leaf = m_index->insertData(currentSubterm,
                                                 m_meaningDictionary);
        FormulaId formulaId = leaf->id;
        auto ret = uniqueFormulaIds.insert(formulaId);
        if (ret.second) {
            types::FormulaPath formulaPath;
            formulaPath.xmlId = xmlId;
            formulaPath.xpath = currentSubterm->getXpath();
            m_formulaDb->insertFormula(leaf->id, crawlId, formulaPath);
            leaf->solutions++;
            numSubExpressions++;
        }
    }

    return numSubExpressions;
}

} }


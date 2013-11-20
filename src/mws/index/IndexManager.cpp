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

#include "IndexManager.hpp"

using namespace std;
using namespace mws;
using namespace mws::types;

namespace mws { namespace index {

IndexManager::IndexManager(dbc::FormulaDb* formulaDb,
                           dbc::CrawlDb* crawlDb,
                           MwsIndexNode* index,
                           MeaningDictionary* meaningDictionary) :
    m_formulaDb(formulaDb), m_crawlDb(crawlDb), m_index(index),
    m_meaningDictionary(meaningDictionary) { }

int
IndexManager::indexContentMath(const types::CmmlToken* cmmlToken,
                               const CrawlData& crawlData) {
    CmmlToken::PtrList::const_reverse_iterator rIt;
    stack<const CmmlToken*> subtermStack;

    int numSubExpressions = 0;

    const CrawlId crawlId = m_crawlDb->putData(crawlData);

    // Using a stack to insert all subterms by
    // going depth first through the CmmlToken
    subtermStack.push(cmmlToken);

    while (!subtermStack.empty())
    {
        // Retrieving the subterm
        // TODO should be const
        const CmmlToken* currentSubterm = subtermStack.top();
        subtermStack.pop();

        // Inserting the children subterms
        for (rIt  = currentSubterm->getChildNodes().rbegin();
             rIt != currentSubterm->getChildNodes().rend();
             rIt ++)
        {
            subtermStack.push(*rIt);
        }

        MwsIndexNode* leaf = m_index->insertData(currentSubterm,
                                                 m_meaningDictionary);

        int ret = m_formulaDb->insertFormula(leaf->id, crawlId,
                                             currentSubterm->getXpath());
        if (ret == 0) {
            leaf->solutions++;
            numSubExpressions++;
        }
    }

    return numSubExpressions;
}

} }


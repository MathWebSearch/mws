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
#ifndef _MWS_INDEX_INDEXMANAGER_HPP
#define _MWS_INDEX_INDEXMANAGER_HPP

/**
  * @file IndexManager.hpp
  * @brief Indexing Manager
  * @date 18 Nov 2013
  */

#include "mws/types/CmmlToken.hpp"
#include "mws/dbc/FormulaDb.hpp"
#include "mws/dbc/CrawlDb.hpp"
#include "mws/index/MwsIndexNode.hpp"

namespace mws { namespace index {

class IndexManager {
private:
    dbc::FormulaDb* m_formulaDb;
    dbc::CrawlDb* m_crawlDb;
    MwsIndexNode* m_index;
    types::MeaningDictionary* m_meaningDictionary;

public:
    IndexManager(dbc::FormulaDb* formulaDb,
                 dbc::CrawlDb* crawlDb,
                 MwsIndexNode* index,
                 types::MeaningDictionary* meaningDictionary);

    /**
     * @brief index content math formula
     * @param cmmlToken ContentMathML node
     * @param crawlData URL and opaque data given in the crawled harvest
     * @return Number of indexed subexpressions on success, -1 on failure.
     */
    virtual int indexContentMath(const types::CmmlToken* cmmlToken,
                                 const types::CrawlData& crawlData);
};

} }

#endif // _MWS_DBC_INDEXMANAGER_HPP

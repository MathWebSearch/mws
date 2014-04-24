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
  * @file MemFormulaDb.cpp
  * @brief Formula Memory Database implementation
  * @date 12 Nov 2013
  */

#include "MemFormulaDb.hpp"

using namespace std;

namespace mws { namespace dbc {

int
MemFormulaDb::insertFormula(const types::FormulaId&   formulaId,
                            const CrawlId&     crawlId,
                            const types::FormulaPath& formulaPath) {
    MemFormulaDb::FormulaInfo formulaInfo;

    formulaInfo.crawlId = crawlId;
    formulaInfo.formulaPath = formulaPath;

    mData[formulaId].push_back(formulaInfo);

    return 0;
}

int
MemFormulaDb::queryFormula(const types::FormulaId &formulaId,
                           unsigned limitMin,
                           unsigned limitSize,
                           QueryCallback queryCallback) {
    auto ret = mData.find(formulaId);
    if (ret == mData.end()) return 0;
    vector<FormulaInfo> formulaInfos = ret->second;
    if (limitMin >= formulaInfos.size()) return 0;
    auto it = formulaInfos.begin() + limitMin;

    for (unsigned i = 0; i < limitSize && it != formulaInfos.end(); i++, it++) {
        if (queryCallback(it->crawlId, it->formulaPath) != 0)
            return -1;
    }

    return 0;
}

} }

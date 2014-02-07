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
 * @file DbQueryManager.cpp
 * @brief DbQueryManager implementation
 * @author cprodescu
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <string>

#include "common/utils/DebugMacros.hpp"
#include "common/utils/macro_func.h"
#include "mws/types/MwsAnswset.hpp"
#include "mws/types/Answer.hpp"

#include "DbQueryManager.hpp"

namespace mws {
namespace dbc {

DbQueryManager::DbQueryManager(CrawlDb* crawlDb, FormulaDb* formulaDb) :
        mCrawlDb(crawlDb), mFormulaDb(formulaDb) {
}

int
DbQueryManager::query(types::FormulaId formulaId,
                      unsigned limitMin,
                      unsigned limitSize,
                      DbAnswerCallback dbAnswerCallback) {
    QueryCallback formulaQueryCallback =
            [dbAnswerCallback, this](const types::CrawlId& crawlId,
                                     const types::FormulaPath& formulaPath) {
        if (crawlId != types::CRAWLID_NULL) {
            return dbAnswerCallback(formulaPath,
                                    this->mCrawlDb->getData(crawlId));
        } else {
            return dbAnswerCallback(formulaPath, types::CRAWLDATA_NULL);
        }
    };
    return mFormulaDb->queryFormula(formulaId, limitMin, limitSize,
                                    formulaQueryCallback);
}

}  // namespace dbc
}  // namespace mws

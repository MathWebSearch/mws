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
 * @brief DbQueryManager declarations
 * @date 2 Dec 2013
 * @author cprodescu
 */
#ifndef _MWS_DBC_DBQUERYMANAGER_HPP
#define _MWS_DBC_DBQUERYMANAGER_HPP

#include <functional>

#include "mws/dbc/CrawlDb.hpp"
#include "mws/dbc/FormulaDb.hpp"

namespace mws {
namespace dbc {

typedef std::function<int (const FormulaPath&, const types::CrawlData&)>
DbAnswerCallback;

class DbQueryManager {
    CrawlDb* mCrawlDb;
    FormulaDb* mFormulaDb;

 public:
    DbQueryManager(CrawlDb* crawlDb, FormulaDb* formulaDb);

    int query(FormulaId formulaId,
              unsigned limitMin,
              unsigned limitSize,
              DbAnswerCallback dbAnswerCallback);

 private:
    DbQueryManager(const DbQueryManager&);
    DbQueryManager& operator=(const DbQueryManager&);
};

}  // namespace dbc
}  // namespace mws

#endif  // _MWS_DBC_DBQUERYMANAGER_HPP

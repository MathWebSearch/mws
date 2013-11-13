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
#ifndef _MWS_DBC_FORMULADB_HPP
#define _MWS_DBC_FORMULADB_HPP

/**
  * @file FormulaDb.hpp
  * @brief Expression Database interface
  * @date 12 Nov 2013
  */

#include <functional>

#include "mws/types/NodeInfo.hpp"

namespace mws { namespace dbc {

typedef std::function<int (const mws::CrawlId&,
                           const mws::FormulaPath&)> QueryCallback;

class FormulaDb {
public:
    virtual ~FormulaDb() {}

    /**
     * @brief insert formula in database
     * @param formulaId id of a leaf node in the index
     * @param crawlId id corresponding to the crawled data
     * @param formulaPath path within the crawled data corresponding to this
     * formula
     * @return 0 on success and -1 on failure.
     */
    virtual int insertFormula(const mws::FormulaId&   formulaId,
                              const mws::CrawlId&     crawlId,
                              const mws::FormulaPath& formulaPath) = 0;

    /**
     * @brief query formula in database
     * @param formulaId id of a leaf node in the index
     * @param limitMin
     * @param limitSize
     * @param queryCallback
     * @return 0 on success and -1 on failure.
     */
    virtual int queryFormula(const mws::FormulaId& formulaId,
                             unsigned limitMin,
                             unsigned limitSize,
                             QueryCallback queryCallback) = 0;
};

} }

#endif // _MWS_DBC_FORMULADB_HPP

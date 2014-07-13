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
#ifndef _MWS_DBC_MEMFORMULADB_HPP
#define _MWS_DBC_MEMFORMULADB_HPP

/**
  * @file MemFormulaDb.hpp
  * @brief Formula Memory Database declarations
  * @date 12 Nov 2013
  */

#include "mws/dbc/FormulaDb.hpp"

#include <map>
#include <vector>

namespace mws { namespace dbc {


class MemFormulaDb : public FormulaDb {
public:
    virtual int insertFormula(const types::FormulaId&   formulaId,
                              const CrawlId&     crawlId,
                              const types::FormulaPath& formulaPath);

    virtual int queryFormula(const types::FormulaId& formulaId,
                             unsigned                limitMin,
                             unsigned                limitSize,
                             QueryCallback           queryCallback);
private:
    struct FormulaInfo {
        CrawlId crawlId;
        types::FormulaPath formulaPath;
    };

    std::map<types::FormulaId, std::vector<FormulaInfo> > mData;
};

} }

#endif // _MWS_DBC_MEMFORMULADB_HPP

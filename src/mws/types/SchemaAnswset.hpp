/*

Copyright (C) 2010-2015 KWARC Group <kwarc.info>

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
#ifndef _SCHEMAANSWSET_HPP
#define _SCHEMAANSWSET_HPP

/**
  * @author Radu Hambasan
  * @date 30 Dec 2014
  */

#include <vector>

#include "mws/types/CmmlToken.hpp"
#include "mws/types/ExprSchema.hpp"
#include "GenericAnswer.hpp"

namespace mws {

struct SchemaAnswset : GenericAnswer {
    std::vector<mws::types::ExprSchema> schemata;
    /// Total number of found schemata (some might have been dropped)
    int total;

    SchemaAnswset() : total(0) {}

    ~SchemaAnswset() {
        for (mws::types::ExprSchema& sch : schemata) {
            delete sch.root;
        }
    }
};

}  // namespace mws

#endif  // _SCHEMAANSWSET_HPP

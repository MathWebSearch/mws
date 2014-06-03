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
  * @brief   File containing the implementation of the writeJsonAnswset
  * function
  * @file    writeJsonAnswset.cpp
  * @author  Corneliu-Claudiu Prodescu
  * @date    30 Jul 2011
  *
  * @edited Radu Hambasan
  * @date 20 Mar 2014
  *
  * License: GPL v3
  *
  * @todo fix writeData return value
  */

#include <cinttypes>

#include "mws/types/FormulaPath.hpp"
using mws::types::FormulaId;
#include "mws/xmlparser/MwsIdsResponseFormatter.hpp"

namespace mws {
namespace parser {

MwsIdsResponseFormatter MwsIdsResponseFormatter::instance;
MwsIdsResponseFormatter* RESPONSE_FORMATTER_MWS_IDS =
    &MwsIdsResponseFormatter::instance;

const char* MwsIdsResponseFormatter::getContentType() const {
    return "application/json";
}

int MwsIdsResponseFormatter::writeData(const MwsAnswset& answerSet,
                                       FILE* output) const {
    fputs("[", output);
    bool should_separate = false;
    for (FormulaId formulaId : answerSet.ids) {
        if (should_separate) fputs(", ", output);
        fprintf(output, "%" PRIformulaId, formulaId);
        should_separate = true;
    }
    fputs("]", output);

    return answerSet.ids.size();
}

}  // namespace parser
}  // namespace mws

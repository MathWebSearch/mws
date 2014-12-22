/*

Copyright (C) 2010-2014 KWARC Group <kwarc.info>

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
#ifndef _SCHEMAENGINE_HPP
#define _SCHEMAENGINE_HPP

/**
  * @author Radu Hambasan <radu.hambasan@gmail.com>
  * @date   22 Dec 2014
  *
  * License: GPL v3
  *
  */

#include <vector>

#include "mws/dbc/DbQueryManager.hpp"
#include "mws/index/ExpressionEncoder.hpp"

#define DEFAULT_DEPTH 3;

namespace mws { namespace query {

struct SchemaOptions {
    uint8_t depth;
    SchemaOptions() : depth(DEFAULT_DEPTH) {}
};

class SchemaEngine {
 public:
    typedef std::vector<encoded_token_t> EncodedFormula;
    SchemaEngine(std::vector<EncodedFormula> formulae, SchemaOptions opt = SchemaOptions());
};

}  // namespace query
}  // namespace mws

#endif //  _SEARCHCONTEXT_HPP

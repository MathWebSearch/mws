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
#include <string>
#include <utility>

#include "mws/index/ExpressionEncoder.hpp"
#include "mws/index/MeaningDictionary.hpp"
#include "mws/index/index.h"
#include "mws/types/CmmlToken.hpp"
#include "mws/types/SchemaAnswset.hpp"
#include "common/utils/util.hpp"
#include "build-gen/config.h"

namespace mws {
namespace query {

constexpr char DEFAULT_QVAR_PREFIX[] = "x";
constexpr uint32_t RETRIEVE_ALL = 0;

typedef std::vector<encoded_token_t> EncodedFormula;

class SchemaEngine {
 public:
    explicit SchemaEngine(const index::MeaningDictionary& meaningDictionary);
    mws::SchemaAnswset* getSchemata(const std::vector<EncodedFormula>& formulae,
                                   uint32_t max_total,
                                   uint8_t depth = DEFAULT_SCHEMA_DEPTH) const;

 private:
    const index::ExpressionDecoder decoder;

    EncodedFormula reduceFormula(const EncodedFormula& expr, uint8_t depth) const;
    size_t completeExpression(const EncodedFormula& expr, size_t startExpr) const;
    std::string hashExpr(const EncodedFormula& expr) const;
    EncodedFormula unhashExpr(const std::string& exprHash) const;
    types::CmmlToken* decodeFormula(const EncodedFormula& expr, uint8_t depth) const;
    std::pair<std::string, std::string> decodeMeaning(
            const types::Meaning& meaning) const;
    ALLOW_TESTER_ACCESS;
};

}  // namespace query
}  // namespace mws

#endif  // _SEARCHCONTEXT_HPP

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
/**
  * @author Radu Hambasan
  * @date 30 Dec 2014
  *
  * License: GPL v3
  */

#include <vector>
using std::vector;

#include "common/utils/compiler_defs.h"
#include "mws/types/Query.hpp"
using mws::types::Query;
#include "mws/types/SchemaAnswset.hpp"
using mws::SchemaAnswset;
#include "mws/types/CmmlToken.hpp"
using mws::types::CmmlToken;
#include "mws/index/ExpressionEncoder.hpp"
using mws::index::HarvestEncoder;
using mws::index::ExpressionEncoder;
#include "mws/index/MeaningDictionary.hpp"
using mws::index::MeaningDictionary;
#include "mws/query/SchemaEngine.hpp"
using mws::query::SchemaEngine;
using mws::query::EncodedFormula;
#include "SchemaQueryHandler.hpp"

namespace mws {
namespace daemon {

SchemaQueryHandler::SchemaQueryHandler(const ExpressionEncoder::Config& conf) :
    _encodingConfig(conf) {}
SchemaQueryHandler::~SchemaQueryHandler() {}

GenericAnswer* SchemaQueryHandler::handleQuery(Query* query) {
    SchemaAnswset* result;
    const uint8_t max_depth = query->max_depth;
    const uint32_t max_total= query->attrResultMaxSize;

    MeaningDictionary dict;
    // using a HarvestEncoder to be able to fill
    // the MeaningDictonary on the fly, so we don't require an index
    HarvestEncoder encoder(&dict);
    vector<EncodedFormula> exprs;
    exprs.reserve(query->tokens.size());

    for (const CmmlToken* tok: query->tokens) {
        EncodedFormula encTok;
        if (encoder.encode(_encodingConfig, tok, &encTok, nullptr) == 0) {
            exprs.push_back(std::move(encTok));
        }
    }

    SchemaEngine schemaEngine(dict);
    result = schemaEngine.getSchemata(exprs, max_total, max_depth);
    return result;
}

}  // namespace daemon
}  // namespace mws

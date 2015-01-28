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
#include <string>
using std::string;

#include "common/utils/compiler_defs.h"
#include "mws/types/Query.hpp"
using mws::types::Query;
#include "mws/types/ExprSchema.hpp"
using mws::types::ExprSchema;
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
    const uint8_t max_depth = query->max_depth;
    const uint32_t max_total = query->attrResultMaxSize;

    MeaningDictionary dict;
    // using a HarvestEncoder to be able to fill
    // the MeaningDictonary on the fly, so we don't require an index for ids
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
    SchemaAnswset* result =
            schemaEngine.getSchemata(exprs, max_total, max_depth);

    /* we can't deduce the substitutions in SchemaEngine because we need
     * the original query, so we will do it here */
    for (ExprSchema& sch : result->schemata) {
        /* As a heuristic,
         * we choose the first formula in this class to be schematized */
        const uint32_t representativeId = sch.formulae.front();
        CmmlToken* exprRoot = query->tokens[representativeId];
        getSubstitutions(exprRoot, sch.root, &sch.subst);
    }

    return result;
}


void SchemaQueryHandler::getSubstitutions(CmmlToken *exprRoot,
                                          CmmlToken *schemaRoot,
                                          vector<string> *subst) {
    if (exprRoot == nullptr || schemaRoot == nullptr || subst == nullptr) {
        PRINT_WARN("nullptr pased. Skipping...");
        return;
    }
    auto exprChildren = exprRoot->getChildNodes();
    auto schemaChildren = exprRoot->getChildNodes();

    if (exprChildren.size() != schemaChildren.size()) {
        PRINT_WARN("Expression and schema are incompatible. Skipping...");
        return;
    }

    // Done with this token
    if (exprChildren.size() == 0) {
        return;
    }

    const string& currSchemaTag = schemaRoot->getTag();
    if (currSchemaTag == types::QVAR_TAG) {
        const string& href = exprRoot->getAttribute("href");
        if (href == "") {
            PRINT_LOG("Missing href attribute. Skipping...");
            return;
        }
        subst->push_back(href);
        return;
    }

    auto expr_it = exprChildren.begin();
    auto schema_it = schemaChildren.begin();
    const string& currExprTag = exprRoot->getTag();
    // disregard the first child of apply
    if (currExprTag == "apply") {
        expr_it++;
        schema_it++;
    }

    /* we know that both exprChildren and schmaChildren have the same length,
     * so it is enough to check for one */
    while (expr_it != exprChildren.end()){
        getSubstitutions(*expr_it, *schema_it, subst);
    }
}

}  // namespace daemon
}  // namespace mws

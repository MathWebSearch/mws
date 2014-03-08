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
 * @file    ExpressionEncoder.cpp
 * @brief   ExpressionEncoder implementation
 *
 * @author  cprodescu
 *
 */

/****************************************************************************/
/* Includes                                                                 */
/****************************************************************************/

#include <stack>
using std::stack;
#include <string>
using std::string;
#include <unordered_map>
using std::unordered_map;
#include <vector>
using std::vector;

#include "mws/index/ExpressionEncoder.hpp"
#include "mws/types/MeaningDictionary.hpp"
using mws::types::MeaningDictionary;
#include "mws/types/MwsSignature.hpp"
using mws::types::MwsSignature;
#include "mws/types/CmmlToken.hpp"
using mws::types::CmmlToken;
using mws::types::Meaning;

/****************************************************************************/
/* Implementation                                                           */
/****************************************************************************/
namespace mws { namespace index {

ExpressionEncoder::ExpressionEncoder(MeaningDictionary* dictionary, MwsSignature* signature) :
    _meaningDictionary(dictionary), _signature(signature) {
}

ExpressionEncoder::~ExpressionEncoder() {}

int
ExpressionEncoder::encode(const CmmlToken* expression,
                          vector<encoded_token_t>* encodedFormula,
                          ExpressionInfo* expressionInfo) {
    int rv = 0;
    stack<const CmmlToken*> dfs_stack;
    MeaningDictionary namedVarDictionary;
    int anonVarId = 0;

    encodedFormula->clear();

    dfs_stack.push(expression);
    while (!dfs_stack.empty()) {
        const CmmlToken* token = dfs_stack.top();
        dfs_stack.pop();
        encoded_token_t encoded_token;
        encoded_token.sort = _signature->getSort("any");

        if (token->isVar()) {
            string qvarName = token->getVarName();
            encoded_token.arity = 1;
            if (qvarName == "") {
                encoded_token.id = _getAnonVarOffset() + anonVarId;
            } else {
                encoded_token.id = _getNamedVarOffset() +
                        namedVarDictionary.put(qvarName);
                if (expressionInfo != NULL) {
                    expressionInfo->qvarNames.push_back(qvarName);
                    expressionInfo->qvarXpaths.push_back(
                                token->getXpathRelative());
                }
            }

            string sortAttribute;
            if ((sortAttribute = token->getAttribute("sort")) != "") {
                types::SortId sortId;
                if ((sortId = _signature->getSort( sortAttribute )) != MwsSignature::SORT_NOT_FOUND) {
                    encoded_token.sort = sortId;
                }
            }
        } else {
            encoded_token.arity = token->getArity();
            encoded_token.id = _getConstantEncoding(token->getMeaning());
            if (encoded_token.id == MeaningDictionary::KEY_NOT_FOUND) {
                rv = -1;
            }
        }
        encodedFormula->push_back(encoded_token);

        // Replenish stack
        for (auto rIt = token->getChildNodes().rbegin();
             rIt != token->getChildNodes().rend(); rIt ++) {
            dfs_stack.push(*rIt);
        }
    }

    return rv;
}

HarvestEncoder::HarvestEncoder(MeaningDictionary *dictionary, MwsSignature* signature) :
    ExpressionEncoder(dictionary, signature) {
}

HarvestEncoder::~HarvestEncoder() {}

MeaningId
HarvestEncoder::_getAnonVarOffset() const {
    return ANON_HVAR_ID_MIN;
}

MeaningId
HarvestEncoder::_getNamedVarOffset() const {
    return HVAR_ID_MIN;
}

MeaningId
HarvestEncoder::_getConstantEncoding(const Meaning& meaning) {
    return CONSTANT_ID_MIN + _meaningDictionary->put(meaning);
}

QueryEncoder::QueryEncoder(MeaningDictionary *dictionary, MwsSignature* signature) :
    ExpressionEncoder(dictionary, signature) {
}

QueryEncoder::~QueryEncoder() {}

MeaningId
QueryEncoder::_getAnonVarOffset() const {
    return ANON_HVAR_ID_MIN;
}

MeaningId
QueryEncoder::_getNamedVarOffset() const {
    return HVAR_ID_MIN;
}

MeaningId
QueryEncoder::_getConstantEncoding(const Meaning& meaning) {
    MeaningId id = _meaningDictionary->get(meaning);

    if (id != MeaningDictionary::KEY_NOT_FOUND) {
        return CONSTANT_ID_MIN + id;
    } else {
        return MeaningDictionary::KEY_NOT_FOUND;
    }
}

}  // namespace index
}  // namespace mws

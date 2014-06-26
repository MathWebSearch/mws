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
#include <stdexcept>
using std::runtime_error;
#include <string>
using std::string;
using std::to_string;
#include <unordered_map>
using std::unordered_map;
#include <vector>
using std::vector;

#include "mws/types/CmmlToken.hpp"
using mws::types::CmmlToken;
using mws::types::Meaning;
#include "mws/index/MeaningDictionary.hpp"
#include "mws/index/ExpressionEncoder.hpp"

/****************************************************************************/
/* Implementation                                                           */
/****************************************************************************/
namespace mws {
namespace index {

ExpressionEncoder::ExpressionEncoder(MeaningDictionary* dictionary)
    : _meaningDictionary(dictionary), _ciTranslationCounter(0) {}

ExpressionEncoder::~ExpressionEncoder() {}

int ExpressionEncoder::encode(const IndexingOptions& options,
                              const CmmlToken* expression,
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

        if (token->isVar()) {
            string qvarName = token->getVarName();
            encoded_token.arity = 1;
            if (qvarName == "") {
                encoded_token.id = _getAnonVarOffset() + anonVarId;
            } else {
                encoded_token.id =
                    _getNamedVarOffset() + namedVarDictionary.put(qvarName);
                if (expressionInfo != nullptr) {
                    expressionInfo->qvarNames.push_back(qvarName);
                    expressionInfo->qvarXpaths.push_back(
                        token->getXpathRelative());
                }
            }
        } else {
            encoded_token.arity = token->getArity();
            if (options.renameCi && token->getTag() == "ci") {
                encoded_token.id = _getCiMeaning((token));
            } else {
                encoded_token.id = _getConstantEncoding(token->getMeaning());
            }
            if (encoded_token.id == MeaningDictionary::KEY_NOT_FOUND) {
                rv = -1;
            }
        }
        encodedFormula->push_back(encoded_token);

        // Replenish stack
        for (auto rIt = token->getChildNodes().rbegin();
             rIt != token->getChildNodes().rend(); rIt++) {
            dfs_stack.push(*rIt);
        }
    }

    return rv;
}

MeaningId ExpressionEncoder::_getCiMeaning(const CmmlToken* token) {
    Meaning tokMeaning = token->getMeaning();
    CmmlToken* tokParent = token->getParentNode();

    // check if we should not rename this ci
    if ((tokMeaning == "#P") || (tokMeaning == "#p") ||
        // the content must have only 1 char:
        (tokMeaning.length() > 2 + token->getTag().length()) ||
        // make sure this is not the 1st child of apply
        ((tokParent != nullptr) && (tokParent->getTag() == "apply") &&
         (tokParent->getChildNodes().front()) == token)) {
        return _getConstantEncoding(tokMeaning);
    }

    auto ciTableIt = _ciTranslations.find(tokMeaning);
    string translatedMeaning;
    if (ciTableIt == _ciTranslations.end()) {
        translatedMeaning = "~" + std::to_string(++_ciTranslationCounter);
        _ciTranslations.insert(make_pair(tokMeaning, translatedMeaning));
    } else {
        translatedMeaning = ciTableIt->second;
    }

    return _getConstantEncoding(translatedMeaning);
}

HarvestEncoder::HarvestEncoder(MeaningDictionary* dictionary)
    : ExpressionEncoder(dictionary) {}

HarvestEncoder::~HarvestEncoder() {}

MeaningId HarvestEncoder::_getAnonVarOffset() const { return ANON_HVAR_ID_MIN; }

MeaningId HarvestEncoder::_getNamedVarOffset() const { return HVAR_ID_MIN; }

MeaningId HarvestEncoder::_getConstantEncoding(const Meaning& meaning) {
    return CONSTANT_ID_MIN + _meaningDictionary->put(meaning);
}

QueryEncoder::QueryEncoder(MeaningDictionary* dictionary)
    : ExpressionEncoder(dictionary) {}

QueryEncoder::~QueryEncoder() {}

MeaningId QueryEncoder::_getAnonVarOffset() const { return ANON_HVAR_ID_MIN; }

MeaningId QueryEncoder::_getNamedVarOffset() const { return HVAR_ID_MIN; }

MeaningId QueryEncoder::_getConstantEncoding(const Meaning& meaning) {
    MeaningId id = _meaningDictionary->get(meaning);

    if (id != MeaningDictionary::KEY_NOT_FOUND) {
        return CONSTANT_ID_MIN + id;
    } else {
        return MeaningDictionary::KEY_NOT_FOUND;
    }
}

ExpressionDecoder::ExpressionDecoder(const MeaningDictionary& dictionary)
    : _lookupTable(dictionary.getReverseLookupTable()) {}

Meaning ExpressionDecoder::getMeaning(MeaningId meaningId) const {
    if (meaningId >= CONSTANT_ID_MIN) {
        return _lookupTable.get(meaningId - CONSTANT_ID_MIN);
    } else if (meaningId >= ANON_QVAR_ID_MIN) {
        return "anonymous_qvar#" + to_string(meaningId - ANON_QVAR_ID_MIN);
    } else if (meaningId >= QVAR_ID_MIN) {
        return "qvar#" + to_string(meaningId - QVAR_ID_MIN);
    } else if (meaningId >= ANON_HVAR_ID_MIN) {
        return "anonymous_hvar#" + to_string(meaningId - ANON_QVAR_ID_MIN);
    } else if (meaningId >= HVAR_ID_MIN) {
        return "hvar#" + to_string(meaningId - HVAR_ID_MIN);
    } else {
        throw runtime_error("Invalid meaningId " + to_string(meaningId));
    }
}

}  // namespace index
}  // namespace mws

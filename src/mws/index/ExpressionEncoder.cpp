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
#include "mws/types/CmmlToken.hpp"
using mws::types::CmmlToken;

/****************************************************************************/
/* Implementation                                                           */
/****************************************************************************/
namespace mws { namespace index {

ExpressionEncoder::ExpressionEncoder(MeaningDictionary* dictionary) :
    _meaningDictionary(dictionary) {
}


int
ExpressionEncoder::encode(const CmmlToken* expression,
                          vector<encoded_token_t>* encodedFormula) {
    stack<const CmmlToken*> dfs_stack;
    MeaningDictionary namedQvarDictionary;
    int anonQvarId = 0;

    encodedFormula->clear();

    dfs_stack.push(expression);
    while (!dfs_stack.empty()) {
        const CmmlToken* token = dfs_stack.top();
        dfs_stack.pop();
        encoded_token_t encoded_token;

        if (token->isQvar()) {
            string qvarName = token->getQvarName();
            encoded_token.arity = 1;
            if (qvarName == "") {
                encoded_token.id = anonQvarId++;
            } else {
                encoded_token.id = namedQvarDictionary.put(qvarName);
            }
        } else {
            encoded_token.arity = token->getArity();
            encoded_token.id = _meaningDictionary->get(token->getMeaning());
            if (encoded_token.id == MeaningDictionary::KEY_NOT_FOUND) {
                return -1;
            }
        }
        encodedFormula->push_back(encoded_token);

        // Replenish stack
        for (auto rIt = token->getChildNodes().rbegin();
             rIt != token->getChildNodes().rend(); rIt ++) {
            dfs_stack.push(*rIt);
        }
    }

    return 0;
}

}  // namespace index
}  // namespace mws

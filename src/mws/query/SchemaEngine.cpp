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
/**
  * @author Radu Hambasan
  * @date 23 Dec 2014
  *
  * License: GPL v3
  *
  */

#include <vector>
using std::vector;
#include <stack>
using std::stack;

#include "mws/index/index.h"
#include "mws/index/MeaningDictionary.hpp"
using mws::index::MeaningDictionary;
#include "mws/query/SchemaEngine.hpp"

namespace mws {
namespace query {
SchemaEngine::SchemaEngine(const MeaningDictionary* meaningDictionary) {
    this->_meaningDict = meaningDictionary;
}

// TODO hashtable
std::vector<types::CmmlToken *>
SchemaEngine::getSchemata(const vector<EncodedFormula> &formulae,
                          uint8_t depth) {

}

EncodedFormula SchemaEngine::reduceFormula(EncodedFormula expr,
                                           uint8_t max_depth) {
    EncodedFormula reducedExpr;
    if (expr.empty()) return {};

    stack<uint32_t> unexplored;
    size_t currToken = 0;
    unexplored.push(expr[currToken].arity);
    currToken++;

    while (currToken < expr.size()) {
        if (unexplored.size() > max_depth) {
            uint32_t exprToComplete = unexplored.top();
            unexplored.pop();
            for (size_t i = 1; i <= exprToComplete; i++) {
                currToken = completeExpression(expr, currToken);
            }
            uint32_t parentExpr = 0;
            while (!unexplored.empty() && (parentExpr == 0)) {
                parentExpr = unexplored.top() - 1;
                unexplored.pop() ;
                if (parentExpr != 0) unexplored.push(parentExpr);
            }
        } else {
            reducedExpr.push_back(expr[currToken]);
            uint32_t ary = expr[currToken].arity;
            if (ary != 0) {
                unexplored.push(ary);
            } else {
                uint32_t parentExpr = 0;
                while (!unexplored.empty() && (parentExpr == 0)) {
                    parentExpr = unexplored.top() - 1;
                    unexplored.pop() ;
                    if (parentExpr != 0) unexplored.push(parentExpr);
                }
            }
            currToken++;
        }
    }

    return reducedExpr;
}

size_t SchemaEngine::completeExpression(EncodedFormula expr,
                                        size_t startExpr) {
    stack<uint32_t> unexplored;
    size_t currTok = startExpr;
    unexplored.push(expr[currTok].arity);
    currTok++;

    while (!unexplored.empty()) {
        uint32_t ary = expr[currTok].arity;
        if (ary != 0) {
            unexplored.push(ary);
        } else {
            uint32_t parentExpr = 0;
            while (!unexplored.empty() && (parentExpr == 0)) {
                parentExpr = unexplored.top() - 1;
                unexplored.pop() ;
                if (parentExpr != 0) unexplored.push(parentExpr);
            }
        }
        currTok++;
    }

    return currTok;
}

}  // namespace query
}  // namespace mws

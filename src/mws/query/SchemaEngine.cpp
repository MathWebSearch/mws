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
#include <string>
using std::string;
#include <sstream>
using std::ostringstream;
using std::istringstream;
#include <unordered_map>
using std::unordered_map;
#include <utility>
using std::pair;
#include <algorithm>

#include "mws/index/index.h"
#include "mws/index/MeaningDictionary.hpp"
using mws::index::MeaningDictionary;
#include "mws/types/CmmlToken.hpp"
using mws::types::CmmlToken;
using mws::types::Meaning;
#include "mws/query/SchemaEngine.hpp"

namespace mws {
namespace query {
SchemaEngine::SchemaEngine(const MeaningDictionary& meaningDictionary)
    : _lookupTable(meaningDictionary.getReverseLookupTable()) {}

vector<CmmlToken*> SchemaEngine::getSchemata(
    const vector<EncodedFormula>& formulae, uint32_t max_total, uint8_t depth) {
    unordered_map<string, int> exprCount;
    for (const EncodedFormula& expr : formulae) {
        string hash = hashExpr(reduceFormula(expr, depth));
        auto it = exprCount.find(hash);
        if (it != exprCount.end()) {
            exprCount[hash] = it->second + 1;
        } else {
            exprCount[hash] = 1;
        }
    }

    vector<pair<string, int>> topExpr(exprCount.begin(), exprCount.end());
    std::sort(topExpr.begin(), topExpr.end(),
              [](const pair<string, int> & p1, const pair<string, int> & p2) {
        return p1.second < p2.second;
    });

    if (max_total < topExpr.size()) {
        topExpr.resize(max_total);
    }

    vector<CmmlToken*> schemata;
    schemata.resize(topExpr.size());
    for (auto& p : topExpr) {
        EncodedFormula e = unhashExpr(p.first);
        schemata.push_back(decodeFormula(e, depth));
    }

    return schemata;
}

EncodedFormula SchemaEngine::reduceFormula(const EncodedFormula& expr,
                                           uint8_t max_depth) {
    EncodedFormula reducedExpr;
    if (expr.empty()) return {};
    if (max_depth == 0) return {};

    stack<uint32_t> unexplored;
    size_t currToken = 0;
    reducedExpr.push_back(expr[currToken]);
    unexplored.push(expr[currToken].arity);
    currToken++;

    while (currToken < expr.size()) {
        if (unexplored.size() >= max_depth) {
            uint32_t exprToComplete = unexplored.top();
            unexplored.pop();
            for (size_t i = 1; i <= exprToComplete; i++) {
                currToken = completeExpression(expr, currToken);
            }
            uint32_t parentExpr = 0;
            while (!unexplored.empty() && (parentExpr == 0)) {
                parentExpr = unexplored.top() - 1;
                unexplored.pop();
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
                    unexplored.pop();
                    if (parentExpr != 0) unexplored.push(parentExpr);
                }
            }
            currToken++;
        }
    }

    return reducedExpr;
}

size_t SchemaEngine::completeExpression(const EncodedFormula& expr,
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
                unexplored.pop();
                if (parentExpr != 0) unexplored.push(parentExpr);
            }
        }
        currTok++;
    }

    return currTok;
}

string SchemaEngine::hashExpr(const EncodedFormula& expr) {
    ostringstream serial;
    serial << expr.size();

    for (const encoded_token_t& tok : expr) {
        serial << tok.id << " " << tok.arity;
    }

    return serial.str();
}

EncodedFormula SchemaEngine::unhashExpr(const std::string& exprHash) {
    istringstream serial(exprHash);
    EncodedFormula expr;

    size_t exprSize;
    serial >> exprSize;
    expr.reserve(exprSize);

    for (size_t i = 0; i < exprSize; i++) {
        uint32_t id;
        uint32_t arity;
        serial >> id >> arity;
        expr.push_back(encoded_token(id, arity));
    }

    return expr;
}

CmmlToken* SchemaEngine::decodeFormula(const EncodedFormula& expr,
                                       uint8_t max_depth) {
    if (expr.size() == 0) {
        CmmlToken* unifZero = CmmlToken::newRoot();
        unifZero->setTag(types::QVAR_TAG);
        unifZero->appendTextContent("x0");
        return unifZero;
    }

    stack<uint32_t> unexplored;
    CmmlToken* currCmml = CmmlToken::newRoot();
    size_t currTok = 0;
    auto meaning = decodeMeaning(_lookupTable.get(expr[currTok].id));
    currCmml->setTag(meaning.first);
    currCmml->appendTextContent(meaning.second);
    unexplored.push(expr[currTok].arity);
    currTok++;

    uint32_t qvar_count = 1;
    while (currTok < expr.size()) {
        if (unexplored.size() >= max_depth) {
            uint32_t anonExprs = unexplored.top();
            unexplored.pop();
            for (size_t i = 1; i <= anonExprs; i++) {
                auto qvar = currCmml->newChildNode();
                qvar->setTag(types::QVAR_TAG);
                qvar->appendTextContent(DEFAULT_QVAR_PREFIX +
                                        std::to_string(qvar_count));
                qvar_count++;
            }
            uint32_t parentExpr = 0;
            while (!unexplored.empty() && (parentExpr == 0)) {
                parentExpr = unexplored.top() - 1;
                unexplored.pop();
                currCmml = currCmml->getParentNode();
                if (parentExpr != 0) unexplored.push(parentExpr);
            }
            currTok++;
        } else {
            currCmml = currCmml->newChildNode();
            meaning = decodeMeaning(_lookupTable.get(expr[currTok].id));
            currCmml->setTag(meaning.first);
            currCmml->appendTextContent(meaning.second);

            uint32_t ary = expr[currTok].arity;
            if (ary != 0) {
                unexplored.push(ary);
            } else {
                uint32_t parentExpr = 0;
                while (!unexplored.empty() && (parentExpr == 0)) {
                    parentExpr = unexplored.top() - 1;
                    unexplored.pop();
                    currCmml = currCmml->getParentNode();
                    if (parentExpr != 0) unexplored.push(parentExpr);
                }
            }
            currTok++;
        }
    }

    return currCmml;
}

pair<string, string> SchemaEngine::decodeMeaning(
    const types::Meaning& meaning) {
    size_t delim_pos = meaning.find('#');
    if (delim_pos == string::npos) {
        return {meaning, ""};
    }

    string tag = meaning.substr(0, delim_pos);
    if (delim_pos + 1 == meaning.size()) {
        return {tag, ""};
    }

    string text = meaning.substr(delim_pos + 1);
    return {tag, text};
}

}  // namespace query
}  // namespace mws

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
#include "mws/types/ExprSchema.hpp"
using mws::types::ExprSchema;
#include "mws/types/SchemaAnswset.hpp"
#include "mws/query/SchemaEngine.hpp"

namespace mws {
namespace query {
SchemaEngine::SchemaEngine(const MeaningDictionary& meaningDictionary)
    : decoder(meaningDictionary) {}

SchemaAnswset* SchemaEngine::getSchemata(
        const vector<EncodedFormula>& formulae, uint32_t max_total,
        uint8_t depth) const {
    SchemaAnswset* result = new SchemaAnswset();

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
    result->total = exprCount.size();

    vector<pair<string, int>> topExpr(exprCount.begin(), exprCount.end());
    std::sort(topExpr.begin(), topExpr.end(),
              [](const pair<string, int> & p1, const pair<string, int> & p2) {
        return p1.second > p2.second;
    });

    // By convention, max_total=0  means "retrieve all schemata"
    if (topExpr.size() > max_total && max_total != 0) {
        topExpr.resize(max_total);
    }

    result->schemata.reserve(topExpr.size());
    for (auto& p : topExpr) {
        EncodedFormula e = unhashExpr(p.first);
        ExprSchema sch;
        sch.root = decodeFormula(e, depth);
        sch.coverage = p.second;
        result->schemata.push_back(sch);
    }

    return result;
}

EncodedFormula SchemaEngine::reduceFormula(const EncodedFormula& expr,
                                           uint8_t max_depth) const {
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
                                        size_t startExpr) const {
    stack<uint32_t> unexplored;
    size_t currTok = startExpr;

    do {
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
    } while (!unexplored.empty());

    return currTok;
}

string SchemaEngine::hashExpr(const EncodedFormula& expr) const {
    ostringstream serial;
    serial << expr.size();

    for (const encoded_token_t& tok : expr) {
        serial << " " << tok.id << " " << tok.arity;
    }

    return serial.str();
}

EncodedFormula SchemaEngine::unhashExpr(const std::string& exprHash) const {
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
                                       uint8_t max_depth) const {
    if (expr.size() == 0) {
        CmmlToken* unifZero = CmmlToken::newRoot();
        unifZero->setTag(types::QVAR_TAG);
        unifZero->appendTextContent("x0");
        return unifZero;
    }

    stack<uint32_t> unexplored;
    CmmlToken* currCmml = CmmlToken::newRoot();
    size_t currTok = 0;
    auto meaning = decodeMeaning(decoder.getMeaning(expr[currTok].id));
    currCmml->setTag(meaning.first);
    currCmml->appendTextContent(meaning.second);
    unexplored.push(expr[currTok].arity);
    currTok++;

    // Corner case when the query consists of a single token with arity 0 */
    if (unexplored.top() == 0) {
        return currCmml;
    }

    uint32_t qvar_count = 1;
    do {
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
        } else {
            currCmml = currCmml->newChildNode();
            meaning = decodeMeaning(decoder.getMeaning(expr[currTok].id));
            currCmml->setTag("m:" + meaning.first);
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
    } while (currTok < expr.size());

    return currCmml;
}

pair<string, string> SchemaEngine::decodeMeaning(
        const types::Meaning& meaning) const {
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

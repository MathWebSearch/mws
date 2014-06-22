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
  * @brief  File containing the implementation of MwsSearchContext Class
  * @file   SearchContext.cpp
  * @author Corneliu-Claudiu Prodescu <c.prodescu@jacobs-university.de>
  * @date   25 May 2011
  *
  * License: GPL v3
  *
  */

#include <list>
using std::list;
#include <map>
using std::map;
#include <utility>
using std::pair;
using std::make_pair;
#include <vector>
using std::vector;

#include "common/utils/ContainerIterator.hpp"
using common::utils::ContainerIterator;
#include "mws/index/encoded_token.h"
#include "mws/index/TmpIndexAccessor.hpp"
using mws::index::TmpIndexAccessor;
#include "mws/index/IndexAccessor.hpp"
using mws::index::IndexAccessor;
#include "mws/types/FormulaPath.hpp"
using mws::types::FormulaPath;
using mws::types::FormulaId;
#include "mws/index/IndexIterator.hpp"
using mws::index::IndexIterator;
#include "mws/dbc/CrawlDb.hpp"
using mws::dbc::CrawlData;
#include "mws/query/SearchContext.hpp"

namespace mws {
namespace query {

template <class Accessor>
struct QvarCtxt {
    IndexIterator<Accessor> iterator;
    // list<typename Accessor::Iterator> backtrackIterators;
    bool isSolved;

    inline QvarCtxt() { isSolved = false; }

    inline typename Accessor::Node* solve(typename Accessor::Index* index,
                                          typename Accessor::Node* root) {
        iterator.set(root, index);
        typename Accessor::Node* node = iterator.next();
        if (node != nullptr) {
            isSolved = true;
        }
        return node;
    }

    inline typename Accessor::Node* nextSol() {
        typename Accessor::Node* node = iterator.next();
        if (node == nullptr) {
            isSolved = false;
        }
        return node;
    }
};

SearchContext::_NodeTriple::_NodeTriple(bool isQvar, MeaningId aMeaningId,
                                        Arity anArity)
    : isQvar(isQvar), meaningId(aMeaningId), arity(anArity) {}

SearchContext::SearchContext(const vector<encoded_token_t>& encodedFormula,
                             const types::Query::Options& options)
    : options(options) {
    map<MeaningId, int> indexedQvars;
    int tokenCount = 0;
    int qvarCount = 0;

    for (encoded_token_t encodedToken : encodedFormula) {
        MeaningId meaningId = encodedToken.id;

        if (encoded_token_is_var(encodedToken)) {           // qvar
            if (encoded_token_is_anon_var(encodedToken)) {  // anonymous qvar
                expr.push_back(_NodeTriple(true, meaningId, qvarCount));
                backtrackPoints.push_back(tokenCount + 1);
                qvarCount++;
            } else {  // named qvar
                auto mapIt = indexedQvars.find(meaningId);
                if (mapIt == indexedQvars.end()) {
                    indexedQvars.insert(make_pair(meaningId, qvarCount));
                    expr.push_back(_NodeTriple(true, meaningId, qvarCount));
                    backtrackPoints.push_back(tokenCount + 1);
                    qvarCount++;
                } else {
                    expr.push_back(_NodeTriple(true, meaningId, mapIt->second));
                }
            }
        } else {  // constant
            expr.push_back(_NodeTriple(false, meaningId, encodedToken.arity));
        }

        tokenCount++;
    }

    mQvarCount = qvarCount;
}

template <class A /* Accessor */>
MwsAnswset* SearchContext::getResult(typename A::Index* index,
                                     dbc::DbQueryManager* dbQueryManager,
                                     unsigned int offset, unsigned int size,
                                     unsigned int maxTotal) {
    // Table containing resolved Qvar and backtrack points
    vector<QvarCtxt<A> > qvarTable;

    auto result = new MwsAnswset;
    size_t currentToken = 0;  // index for the expression vector
    unsigned int found = 0;   // # of found matches
    int lastSolvedQvar = -1;  // last qvar that was solved
    typename A::Node* currentNode = A::getRootNode(index);

    // Checking the arguments
    if (offset + size > maxTotal) {
        if (maxTotal <= offset) {
            size = 0;
        } else {
            size = maxTotal - offset;
        }
    }

    // Initializing the qvarTable
    qvarTable.resize(mQvarCount);

    // Retrieving the solutions
    while (found < maxTotal) {
        // By default not backtracking
        bool backtrack = false;

        // Evaluating current token and deciding if to go ahead or backtrack
        if (currentToken < expr.size()) {
            if (expr[currentToken].isQvar) {
                int qvarId = expr[currentToken].arity;
                if (qvarTable[qvarId].isSolved) {
                    for (auto& elem : qvarTable[qvarId].iterator.getPath()) {
                        encoded_token_t token = A::getToken(elem);
                        currentNode = A::getChild(index, currentNode, token);
                        if (currentNode == nullptr) {
                            backtrack = true;
                            break;
                        }
                    }
                } else {
                    currentNode = qvarTable[qvarId].solve(index, currentNode);
                    if (currentNode != nullptr) {
                        lastSolvedQvar = qvarId;
                    } else {
                        backtrack = true;
                    }
                }
            } else {
                encoded_token_t token = encoded_token(
                    expr[currentToken].meaningId, expr[currentToken].arity);
                currentNode = A::getChild(index, currentNode, token);
                if (currentNode == nullptr) {
                    backtrack = true;
                }
            }
        } else {
            // Handling the solutions
            size_t hitsCount;
            if (options.includeHits) {
                hitsCount = A::getHitsCount(currentNode);
            } else {
                hitsCount = 1;
            }

            if (found < size + offset && found + hitsCount > offset) {
                if (options.includeHits) {
                    FormulaId formulaId = A::getFormulaId(currentNode);
                    unsigned dbOffset;
                    unsigned dbMaxSize;
                    if (offset < found) {
                        dbOffset = 0;
                        dbMaxSize = size + offset - found;
                    } else {
                        dbOffset = offset - found;
                        dbMaxSize = size;
                    }

                    auto callback = [result](const FormulaPath& formulaPath,
                                             const CrawlData& crawlData) {
                        auto answer = new mws::types::Answer();
                        answer->data = crawlData;
                        answer->uri = formulaPath.xmlId;
                        answer->xpath = formulaPath.xpath;
                        result->answers.push_back(answer);
                        return 0;
                    };

                    dbQueryManager->query(formulaId, dbOffset, dbMaxSize,
                                          callback);
                }
                if (options.includeMwsIds) {
                    result->ids.insert(A::getFormulaId(currentNode));
                }
            }

            found += hitsCount;

            // making sure we haven't surpassed maxTotal
            if (found > maxTotal) found = maxTotal;

            // backtracking to the next
            backtrack = true;
        }

        if (backtrack) {
            // Backtracking or going to the next expression token
            // starting with the last
            while (lastSolvedQvar >= 0 &&
                   nullptr ==
                       (currentNode = qvarTable[lastSolvedQvar].nextSol())) {
                lastSolvedQvar--;
            }

            if (lastSolvedQvar == -1) {
                // No more solutions
                break;
            } else {
                currentToken = backtrackPoints[lastSolvedQvar];
            }
        } else {
            currentToken++;
        }
    }
    result->total = found;

    return result;
}

// Declare specializations

template MwsAnswset* SearchContext::getResult<TmpIndexAccessor>(
    TmpIndexAccessor::Index* index, dbc::DbQueryManager* dbQueryManger,
    unsigned int offset, unsigned int size, unsigned int maxTotal);

template MwsAnswset* SearchContext::getResult<IndexAccessor>(
    IndexAccessor::Index* index, dbc::DbQueryManager* dbQueryManger,
    unsigned int offset, unsigned int size, unsigned int maxTotal);

}  // namespace query
}  // namespace mws

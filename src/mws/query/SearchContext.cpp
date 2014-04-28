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

#include "mws/index/encoded_token.h"
#include "mws/index/TmpIndexAccessor.hpp"
using mws::index::TmpIndexAccessor;
#include "mws/index/IndexAccessor.hpp"
using mws::index::IndexAccessor;
#include "mws/types/FormulaPath.hpp"
using mws::types::FormulaPath;
using mws::types::FormulaId;
#include "mws/dbc/CrawlDb.hpp"
using mws::dbc::CrawlData;
#include "mws/query/SearchContext.hpp"

namespace mws {
namespace query {

template<class Accessor>
struct qvarCtxt {
    typedef typename Accessor::Iterator MapIterator;

    list<pair<MapIterator, MapIterator> > backtrackIterators;
    bool isSolved;

    inline qvarCtxt() {
        isSolved = false;
    }

    inline typename Accessor::Node* solve(typename Accessor::Index* index,
                                          typename Accessor::Node* node) {
        int totalArrity = 1;
        while (totalArrity > 0) {
            typename Accessor::Iterator begin =
                    Accessor::getChildrenBegin(node);
            typename Accessor::Iterator end =
                    Accessor::getChildrenEnd(node);
            if (begin == end) {
                backtrackIterators.clear();
                return NULL;
            }
            backtrackIterators.push_back(std::make_pair(begin, end));
            totalArrity += Accessor::getArity(begin) - 1;
            node = Accessor::getNode(index, begin);
        }

        isSolved = true;
        return node;
    }

    inline typename Accessor::Node* nextSol(typename Accessor::Index* index) {
        int totalArrity = 0;
        totalArrity -= Accessor::getArity(backtrackIterators.back().first) - 1;

        backtrackIterators.back().first++;
        while (backtrackIterators.back().first ==
               backtrackIterators.back().second) {
            backtrackIterators.pop_back();
            if (backtrackIterators.empty()) {
                isSolved = false;
                return NULL;
            }
            totalArrity +=
                    1 - Accessor::getArity(backtrackIterators.back().first);
            backtrackIterators.back().first++;
        }
        totalArrity += Accessor::getArity(backtrackIterators.back().first) - 1;
        // Found a valid next, now we need to complete it to the right arrity
        typename Accessor::Node* currentNode =
                Accessor::getNode(index, backtrackIterators.back().first);
        while (totalArrity) {
            typename Accessor::Iterator begin =
                    Accessor::getChildrenBegin(currentNode);
            typename Accessor::Iterator end =
                    Accessor::getChildrenEnd(currentNode);
            backtrackIterators.push_back(std::make_pair(begin, end));
            // Updating currentNode and arrity
            currentNode = Accessor::getNode(index, begin);
            totalArrity += Accessor::getArity(begin) - 1;
        }
        // We have selected a different solution
        return currentNode;
    }
};

SearchContext::NodeTriple::
NodeTriple(bool isQvar, MeaningId aMeaningId, Arity anArity)
    : isQvar(isQvar), meaningId(aMeaningId), arity(anArity) {
}

SearchContext::
SearchContext(const vector<encoded_token_t>& encodedFormula) {
    map<MeaningId, int> indexedQvars;
    int tokenCount = 0;
    int qvarCount  = 0;

    for (encoded_token_t encodedToken : encodedFormula) {
        MeaningId meaningId = encodedToken.id;

        if (encoded_token_is_var(encodedToken)) {  // qvar
            if (encoded_token_is_anon_var(encodedToken)) {  // anonymous qvar
                expr.push_back(NodeTriple(true, meaningId, qvarCount));
                backtrackPoints.push_back(tokenCount+1);
                qvarCount++;
            } else {  // named qvar
                auto mapIt = indexedQvars.find(meaningId);
                if (mapIt == indexedQvars.end()) {
                    indexedQvars.insert(make_pair(meaningId, qvarCount));
                    expr.push_back(NodeTriple(true, meaningId, qvarCount));
                    backtrackPoints.push_back(tokenCount+1);
                    qvarCount++;
                } else {
                    expr.push_back(NodeTriple(true, meaningId, mapIt->second));
                }
            }
        } else {  // constant
            expr.push_back(NodeTriple(false, meaningId, encodedToken.arity));
        }

        tokenCount++;
    }

    mQvarCount = qvarCount;
}


SearchContext::~SearchContext()
{
    // Nothing to do here
}

template<class A /* Accessor */>
MwsAnswset*
SearchContext::getResult(typename A::Index* index,
                         dbc::DbQueryManager* dbQueryManger,
                         unsigned int offset,
                         unsigned int size,
                         unsigned int maxTotal) {
    // Table containing resolved Qvar and backtrack points
    vector<qvarCtxt<A> > qvarTable;

    MwsAnswset* result = new MwsAnswset;
    size_t currentToken = 0;            // index for the expression vector
    unsigned int  found = 0;            // # of found matches
    int lastSolvedQvar = -1;            // last qvar that was solved
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
                    for (auto it = qvarTable[qvarId].backtrackIterators.begin();
                         it != qvarTable[qvarId].backtrackIterators.end();
                         it ++) {
                        encoded_token_t token = A::getToken(it->first);
                        currentNode = A::getChild(index, currentNode, token);
                        if (currentNode == NULL) {
                            backtrack = true;
                            break;
                        }
                    }
                } else {
                    currentNode = qvarTable[qvarId].solve(index, currentNode);
                    if (currentNode != NULL) {
                        lastSolvedQvar = qvarId;
                    } else {
                        backtrack = true;
                    }
                }
            } else {
                encoded_token_t token =
                        encoded_token(expr[currentToken].meaningId,
                                      expr[currentToken].arity);
                currentNode = A::getChild(index, currentNode, token);
                if (currentNode == NULL) {
                    backtrack = true;
                }
            }
        } else {
            // Handling the solutions
            if (found < size + offset &&
                found + A::getHitsCount(currentNode) > offset)
            {
                unsigned dbOffset;
                unsigned dbMaxSize;
                if (offset < found) {
                    dbOffset = 0;
                    dbMaxSize = size + offset - found;
                } else {
                    dbOffset = offset - found;
                    dbMaxSize = size;
                }
                dbc::DbAnswerCallback callback =
                        [result](const FormulaPath& formulaPath,
                                 const CrawlData& crawlData) {
                    mws::types::Answer* answer = new mws::types::Answer();
                    answer->data = crawlData;
                    answer->uri = formulaPath.xmlId;
                    answer->xpath = formulaPath.xpath;
                    result->answers.push_back(answer);
                    return 0;
                };

                dbQueryManger->query(A::getFormulaId(currentNode), dbOffset,
                                     dbMaxSize, callback);
            }

            found += A::getHitsCount(currentNode);

            // making sure we haven't surpassed maxTotal
            if (found > maxTotal) found = maxTotal;

            // backtracking to the next
            backtrack = true;
        }

        if (backtrack) {
            // Backtracking or going to the next expression token
            // starting with the last
            while (lastSolvedQvar >= 0 &&
                    NULL == (currentNode =
                             qvarTable[lastSolvedQvar].nextSol(index)))
            {
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

template MwsAnswset*
SearchContext::
getResult<TmpIndexAccessor>(TmpIndexAccessor::Index* index,
dbc::DbQueryManager* dbQueryManger,
unsigned int offset,
unsigned int size,
unsigned int maxTotal);

template MwsAnswset*
SearchContext::
getResult<IndexAccessor>(IndexAccessor::Index* index,
dbc::DbQueryManager* dbQueryManger,
unsigned int offset,
unsigned int size,
unsigned int maxTotal);

}  // namespace query
}  // namespace mws

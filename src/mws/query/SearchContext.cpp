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

#include <map>
using std::map;
#include <utility>
using std::make_pair;
#include <vector>
using std::vector;

#include "SearchContext.hpp"
#include "mws/index/encoded_token.h"
#include "mws/types/NodeInfo.hpp"
#include "mws/types/MwsSignature.hpp"
using mws::types::FormulaId;
using mws::types::SortId;

namespace mws { namespace query {

SearchContext::SearchContext(const vector<encoded_token_t>& encodedFormula) {
    map<MeaningId, int> indexedQvars;
    int tokenCount = 0;
    int qvarCount  = 0;

    for (encoded_token_t encodedToken : encodedFormula) {
        MeaningId meaningId = encodedToken.id;

        if (encoded_token_is_var(encodedToken)) {  // qvar
            SortId sortId = encodedToken.sort;
            if (encoded_token_is_anon_var(encodedToken)) {  // anonymous qvar
                expr.push_back(
                        makeNodeTriple(true,
                                       meaningId,
                                       qvarCount)
                        );
                qvarSorts.insert( make_pair(qvarCount, sortId) );
                backtrackPoints.push_back(tokenCount+1);
                qvarCount++;
            } else {  // named qvar
                auto mapIt = indexedQvars.find(meaningId);
                if (mapIt == indexedQvars.end()) {
                    indexedQvars.insert(make_pair(meaningId, qvarCount));
                    expr.push_back(
                            makeNodeTriple(true,
                                           meaningId,
                                           qvarCount)
                            );
                    qvarSorts.insert( make_pair(qvarCount, sortId) );
                    backtrackPoints.push_back(tokenCount+1);
                    qvarCount++;
                } else {
                    expr.push_back(
                            makeNodeTriple(true,
                                           meaningId,
                                           mapIt->second)
                            );
                    map<int, SortId>::iterator it = qvarSorts.find( mapIt->second );
                    if (it != qvarSorts.end()) {
                        if (mws::types::subsetOf(sortId, it->second)) {
                            it->second = sortId;
                        }
                    } else {
                        qvarSorts.insert( make_pair(qvarCount, sortId) );
                    }
                }
            }
        } else {  // constant
            expr.push_back(
                    makeNodeTriple(false,
                                   meaningId,
                                   encodedToken.arity)
                    );
        }

        tokenCount++;
    }

    // Initializing the qvarTable
    qvarTable.resize(qvarCount);
}


SearchContext::~SearchContext()
{
    // Nothing to do here
}


MwsAnswset*
SearchContext::getResult(MwsIndexNode* data,
                         dbc::DbQueryManager* dbQueryManger,
                         unsigned int offset,
                         unsigned int size,
                         unsigned int maxTotal)
{
    MwsAnswset*   result;
    MwsIndexNode* currentNode;
    MwsIndexNode::_MapType::iterator mapIt;
    int           currentToken;     // iterator for the expr
    int           exprSize;
    unsigned int  found;            // # of found matches
    int           lastSolvedQvar;
    bool          backtrack;

    // Initializing variables
    result             = new MwsAnswset();
    found              = 0;
    exprSize           = expr.size();

    currentToken   = 0;      // Current token from expr vector
    currentNode    = data;   // Current MwsIndexNode
    lastSolvedQvar = -1;     // Last qvar that was solved

    // Checking the arguments
    if (offset + size > maxTotal)
    {
        if (maxTotal <= offset)
        {
            size = 0;
        }
        else
        {
            size = maxTotal - offset;
        }
    }

    // Retrieving the solutions
    while (found < maxTotal)
    {
        // By default not backtracking
        backtrack = false;

        // Evaluating current token and deciding if to go ahead or backtrack
        if (currentToken < exprSize)
        {
            if (expr[currentToken].isQvar)
            {
                int qvarId = expr[currentToken].arity;
                if (qvarTable[qvarId].isSolved)
                {
                    std::list<
                        std::pair<qvarCtxt::mapIteratorType,
                                  qvarCtxt::mapIteratorType>
                        > :: iterator it;
                    for (it  = qvarTable[qvarId].backtrackIterators.begin();
                         it != qvarTable[qvarId].backtrackIterators.end();
                         it ++)
                    {
                        mapIt = currentNode->children.find(it->first->first);
                        if (mapIt != currentNode->children.end())
                        {
                            currentNode = mapIt->second;
                        }
                        else
                        {
                            backtrack = true;
                            break;
                        }
                    }
                }
                else
                {
                    currentNode = qvarTable[qvarId].solve(currentNode);
                    if (currentNode)
                    {
                        lastSolvedQvar = qvarId;
                    }
                    else
                    {
                        backtrack = true;
                    }
                }
            }
            else
            {
                mapIt = currentNode->children.find(
                        make_pair(expr[currentToken].meaningId,
                                  expr[currentToken].arity));
                if (mapIt != currentNode->children.end())
                {
                    currentNode = mapIt->second;
                }
                else
                {
                    backtrack = true;
                }
            }
        }
        else
        {
            // Handling the solutions
            if (found < size + offset &&
                found + currentNode->solutions > offset)
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
                        [result](const types::FormulaPath& formulaPath,
                                 const types::CrawlData& crawlData) {
                    mws::types::Answer* answer = new mws::types::Answer();
                    answer->data = crawlData;
                    answer->uri = formulaPath.xmlId;
                    answer->xpath = formulaPath.xpath;
                    result->answers.push_back(answer);
                    return 0;
                };

                dbQueryManger->query((FormulaId)currentNode->id, dbOffset,
                                     dbMaxSize, callback);
            }

            found += currentNode->solutions;

            // making sure we haven't surpassed maxTotal
            if (found > maxTotal) found = maxTotal;

            // backtracking to the next
            backtrack = true;
        }

        if (backtrack)
        {
            // Backtracking or going to the next expression token
            // starting with the last
            while (lastSolvedQvar >= 0 &&
                    NULL == (currentNode = qvarTable[lastSolvedQvar].nextSol()))
            {
                lastSolvedQvar--;
            }

            if (lastSolvedQvar == -1)
            {
                // No more solutions
                break;
            }
            else
            {
                currentToken = backtrackPoints[lastSolvedQvar];
            }
        }
        else
        {
            currentToken++;
        }
    }
    result->total = found;

    return result;
}

}  // namespace query
}  // namespace mws

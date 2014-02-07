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

// System includes


// Local includes

#include "SearchContext.hpp"

// Namespaces

using namespace std;
using namespace mws;
using namespace mws::types;


SearchContext::SearchContext(CmmlToken* expression, MeaningDictionary* dict)
{
    int                                      tokenCount;
    map<std::string, int>                    indexedQvars;
    map<std::string, int> :: iterator        mapIt;
    CmmlToken::PtrList::const_reverse_iterator rIt;
    stack<CmmlToken*>                        tokenStack;
    CmmlToken*                               currentToken;
    int                                      qvarCount;

    tokenCount = 0;
    qvarCount  = 0;

    tokenStack.push(expression);
    while (!tokenStack.empty())
    {
        currentToken = tokenStack.top();
        tokenStack.pop();
        MeaningId meaning_id = dict->get(currentToken->getMeaning());

        // Pushing the children on the stack (in reverse order to maintain DFS)
        for (rIt  = currentToken->getChildNodes().rbegin();
             rIt != currentToken->getChildNodes().rend();
             rIt ++)
        {
            tokenStack.push(*rIt);
        }

        // Checking if Qvar
        if (currentToken->isQvar())
        {
            // Checking if qvar was not previously indexed
            if (currentToken->getQvarName() == "")
            {
                expr.push_back(
                        makeNodeTriple(true,
                                       meaning_id,
                                       qvarCount)
                        );
                backtrackPoints.push_back(tokenCount+1);
                qvarCount++;
            }
            else
            {
                mapIt = indexedQvars.find(currentToken->getQvarName());
                if (mapIt == indexedQvars.end())
                {
                    indexedQvars.insert(make_pair(currentToken->getQvarName(),
                                                  qvarCount));
                    expr.push_back(
                            makeNodeTriple(true,
                                           meaning_id,
                                           qvarCount)
                            );
                    backtrackPoints.push_back(tokenCount+1);
                    qvarCount++;
                    // Name / xpath book keeping
                    qvarNames.push_back(currentToken->getTextContent());
                    qvarXpaths.push_back(currentToken->getXpathRelative());
                }
                else
                {
                    expr.push_back(
                            makeNodeTriple(true,
                                           meaning_id,
                                           mapIt->second)
                            );
                }
            }
        }
        // Otherwise just pushing the element in the DFS
        else
        {
            expr.push_back(
                    makeNodeTriple(false,
                                   meaning_id,
                                   currentToken->getChildNodes().size())
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
    result->qvarNames  = qvarNames;
    result->qvarXpaths = qvarXpaths;
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
                    NULL == (currentNode = qvarTable[lastSolvedQvar].nextSol()) )
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


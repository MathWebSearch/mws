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
#ifndef _SEARCHCONTEXTTYPES_HPP
#define _SEARCHCONTEXTTYPES_HPP

/**
  * @brief  File containing auxiliary types of SearchContext Class.
  * @file   SearchContextTypes.hpp
  * @author Corneliu-Claudiu Prodescu <c.prodescu@jacobs-university.de>
  * @date   25 May 2011
  *
  * License: GPL v3
  *
  */

// System includes

#include <list>
#include <map>
#include <string>
#include <utility>                     // STL pair
#include <vector>

// Local includes

#include "mws/types/CmmlToken.hpp"     // CmmlToken class header
#include "mws/types/MwsSignature.hpp"  // Mws Signature and solver
#include "mws/types/MwsAnswset.hpp"    // MWS Answer set class header
#include "mws/index/MwsIndexNode.hpp"  // MWS Index node class header
#include "common/utils/DebugMacros.hpp"

// Typedefs

namespace mws
{


struct nodeTriple
{
    bool           isQvar;
    mws::MeaningId meaningId;
    mws::Arity     arity;
    mws::SortId sort;

};

inline struct nodeTriple
makeNodeTriple(bool isQvar, mws::MeaningId aMeaningId, mws::Arity anArity, mws::SortId aSort)
{
    struct nodeTriple result;

    result.isQvar    = isQvar;
    result.meaningId = aMeaningId;
    result.arity     = anArity;
    result.sort      = aSort;

    return result;
}

struct qvarCtxt
{
    typedef
    mws::MwsIndexNode::_MapType::iterator
    mapIteratorType;

    std::list<
        std::pair<
            mapIteratorType,
            mapIteratorType
        >
    > backtrackIterators;
    bool isSolved;

    inline qvarCtxt()
    {
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_IN;
#endif

        isSolved = false;

#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_OUT;
#endif
    }

    inline mws::MwsIndexNode* solve(mws::MwsIndexNode* node)
    {
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_IN;
#endif

        int totalArrity;
        std::pair<
            mapIteratorType,
            mapIteratorType> currentPair;

        totalArrity = 1;

        while (totalArrity)
        {
            currentPair = make_pair(node->children.begin(),
                                    node->children.end());
            if (currentPair.first == currentPair.second)
            {
                backtrackIterators.clear();
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_OUT;
#endif
                return NULL;
            }
            totalArrity += currentPair.first->first.second.first - 1;
            backtrackIterators.push_back(currentPair);
            node = currentPair.first->second;
        }

        isSolved = true;
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_OUT;
#endif
        return node;
    }

    inline mws::MwsIndexNode* nextSol()
    {
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_IN;
#endif
        int totalArrity;
        mws::MwsIndexNode* currentNode;
        std::pair<
            mapIteratorType,
            mapIteratorType> currentPair;

        totalArrity = 0;
        totalArrity -= backtrackIterators.back().first->first.second.first - 1;

        backtrackIterators.back().first++;
        while (backtrackIterators.back().first ==
                backtrackIterators.back().second)
        {
            backtrackIterators.pop_back();
            if (backtrackIterators.empty())
            {
                isSolved = false;
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_OUT;
#endif
                return NULL;
            }
            totalArrity += 1 - backtrackIterators.back().first->first.second.first;
            backtrackIterators.back().first++;
        }
        totalArrity += backtrackIterators.back().first->first.second.first - 1;
        // Found a valid next, now we need to complete it to the right arrity
        currentNode = backtrackIterators.back().first->second;
        while (totalArrity)
        {
            currentPair = make_pair(currentNode->children.begin(),
                                    currentNode->children.end());
            backtrackIterators.push_back(currentPair);
            // Updating currentNode and arrity
            currentNode = currentPair.first->second;
            totalArrity += currentPair.first->first.second.first - 1;
        }
        // We have selected a different solution
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_OUT;
#endif
        return currentNode;
    }

    /** @brief Method to determine if qvar is replaced by a formula of correct sort
      * @param signature The signature of the math formulae
      * @param sortId the required sort of the query variable
      * @return 1 if the replacing formula has the required sort, 0 otherwise
      */
    inline int isSolutionSorted(mws::types::MwsSignature* signature,
                                mws::SortId sortId)
    {
        if (sortId == 1)
            return 1;

        std::list<
            std::pair<qvarCtxt::mapIteratorType,
                      qvarCtxt::mapIteratorType>
            > :: iterator it;

        std::stack< nodeTriple > sortStack;
        std::stack< nodeTriple > auxStack;
        std::stack< int > arityLeft;
        std::vector< std::pair<mws::MeaningId, mws::SortId> > functionApplication;

        for(it = backtrackIterators.begin(); it != backtrackIterators.end(); it ++ ) {
            NodeInfo nodeInfo = it->first->first;
            Arity nodeArity = nodeInfo.second.first;
            SortId nodeSort = nodeInfo.second.second;
            sortStack.push(makeNodeTriple(false,
                nodeInfo.first, nodeArity, nodeSort));

            if (nodeArity == 0) {
                bool subtractedArity = false;
                while( !arityLeft.empty() && !subtractedArity ) {
                    int arity = arityLeft.top();
                    arityLeft.pop();
                    if (arity > 1) {
                        arityLeft.push( arity - 1 );
                        subtractedArity = true;
                    } else {
                        // We can replace a function application by a sort
                        bool hasPositiveArity = false;
                        do {
                            auxStack.push( sortStack.top() );
                            sortStack.pop();
                            if (auxStack.top().arity > 0) {
                                hasPositiveArity = true;
                            }
                        } while (!sortStack.empty() && !hasPositiveArity);
                        // Set up the function application of sorts
                        functionApplication.clear();
                        while (!auxStack.empty()) {
                            nodeTriple node = auxStack.top();
                            std::pair<mws::MeaningId, mws::SortId> currentPair = std::make_pair(node.meaningId, node.sort);
                            functionApplication.push_back(currentPair);
                            auxStack.pop();
                        }
                        mws::SortId newSortId = signature->getSortFunctionApplication(functionApplication);
                        sortStack.push( makeNodeTriple(false, 0, 0, newSortId) );
                    }
                }
            } else {
                arityLeft.push( nodeArity );
            }
        }
        nodeTriple result = sortStack.top();
        sortStack.pop();
        return (mws::types::subsetOf(result.sort, sortId));
    }
};

}

#endif // _SEARCHCONTEXTTYPES_HPP

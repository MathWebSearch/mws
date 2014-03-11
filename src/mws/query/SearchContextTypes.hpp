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
#include <iostream>

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

};

inline struct nodeTriple
makeNodeTriple(bool isQvar, mws::MeaningId aMeaningId, mws::Arity anArity)
{
    struct nodeTriple result;

    result.isQvar    = isQvar;
    result.meaningId = aMeaningId;
    result.arity     = anArity;

    return result;
}

struct nodeTripleExtended : public nodeTriple {
    mws::types::SortId sort;
};

inline struct nodeTripleExtended
makeNodeTripleExtended(mws::MeaningId aMeaningId, mws::Arity anArity, mws::types::SortId aSort)
{
    struct nodeTripleExtended result;

    result.isQvar    = false;
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
            totalArrity += currentPair.first->first.second - 1;
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
        totalArrity -= backtrackIterators.back().first->first.second - 1;

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
            totalArrity += 1 - backtrackIterators.back().first->first.second;
            backtrackIterators.back().first++;
        }
        totalArrity += backtrackIterators.back().first->first.second - 1;
        // Found a valid next, now we need to complete it to the right arrity
        currentNode = backtrackIterators.back().first->second;
        while (totalArrity)
        {
            currentPair = make_pair(currentNode->children.begin(),
                                    currentNode->children.end());
            backtrackIterators.push_back(currentPair);
            // Updating currentNode and arrity
            currentNode = currentPair.first->second;
            totalArrity += currentPair.first->first.second - 1;
        }
        // We have selected a different solution
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_OUT;
#endif
        return currentNode;
    }

    inline int isSolutionSorted(mws::types::MwsSignature* signature,
                                mws::types::SortId sortId)
    {
        if (sortId == 1)
            return 1;

        std::list<
            std::pair<qvarCtxt::mapIteratorType,
                      qvarCtxt::mapIteratorType>
            > :: iterator it;

        std::stack< nodeTripleExtended > sortStack;
        std::stack< int > arityLeft;

        std::stack< nodeTripleExtended > auxStack;
        std::vector< std::pair<mws::MeaningId, mws::types::SortId> > functionApplication;

        // std::cout << "item: ";
        // for(it = backtrackIterators.begin(); it != backtrackIterators.end(); it ++ ) {
        //     std::cout << (int)it->first->first.first << " ";
        // }
        // std::cout << "\n";

        // std::cout << "Formula " << (int)signature->getSmallestSort() << "\n";
        for(it = backtrackIterators.begin(); it != backtrackIterators.end(); it ++ ) {
            NodeInfo nodeInfo = it->first->first;
            sortStack.push(makeNodeTripleExtended(
                nodeInfo.first, nodeInfo.second,
                (nodeInfo.second == 0)?signature->getSmallestSort():0 ));
            if (nodeInfo.second > 0) {
                arityLeft.push( nodeInfo.second );
            } else {
                bool subtractedArity = false;
                while( !arityLeft.empty() && !subtractedArity ) {
                    int arity = arityLeft.top();
                    arityLeft.pop();
                    if (arity > 1) {
                        arityLeft.push( arity - 1 );
                        subtractedArity = true;
                    } else {
                        // Evaluate expression
                        // Pop until you get something with arity > 0
                        // Evaluate the (apply) (func) (x) ... only to get the correct sort
                        bool hasPositiveArity = false;
                        do {
                            auxStack.push( sortStack.top() );
                            sortStack.pop();
                            if (auxStack.top().arity > 0) {
                                hasPositiveArity = true;
                            }
                        } while (!sortStack.empty() && !hasPositiveArity);
                        functionApplication.clear();
                        while (!auxStack.empty()) {
                            nodeTripleExtended node = auxStack.top();
                            std::pair<mws::MeaningId, mws::types::SortId> currentPair = std::make_pair(node.meaningId, node.sort);
                            functionApplication.push_back(currentPair);
                            auxStack.pop();
                        }
                        mws::types::SortId newSortId = signature->getSortFunctionApplication(functionApplication);
                        sortStack.push( makeNodeTripleExtended(0, 0, newSortId) );
                    }
                }
            }
        }
        nodeTripleExtended result = sortStack.top();
        sortStack.pop();
        // std::cout << "item of sort ";
        // std::cout << (int)result.sort << " needing sort " << (int)sortId << "\n";
        return (mws::types::subsetOf(result.sort, sortId));
    }
};

}

#endif // _SEARCHCONTEXTTYPES_HPP

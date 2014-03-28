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
#include "mws/types/MwsAnswset.hpp"    // MWS Answer set class header
#include "mws/index/MwsIndexNode.hpp"  // MWS Index node class header

namespace mws
{

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

    inline qvarCtxt() {
        isSolved = false;
    }

    inline mws::MwsIndexNode* solve(mws::MwsIndexNode* node) {
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
                return NULL;
            }
            totalArrity += currentPair.first->first.second - 1;
            backtrackIterators.push_back(currentPair);
            node = currentPair.first->second;
        }

        isSolved = true;
        return node;
    }

    inline mws::MwsIndexNode* nextSol()
    {
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
        return currentNode;
    }
};

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

}

#endif // _SEARCHCONTEXTTYPES_HPP

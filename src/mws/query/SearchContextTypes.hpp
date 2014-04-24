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
#include <utility>
#include <vector>

// Local includes

#include "mws/types/CmmlToken.hpp"     // CmmlToken class header
#include "mws/types/MwsAnswset.hpp"    // MWS Answer set class header
#include "mws/index/MwsIndexNode.hpp"  // MWS Index node class header

namespace mws {

template<class Accessor>
struct qvarCtxt {
    typedef typename Accessor::Iterator MapIterator;

    std::list<std::pair<MapIterator,MapIterator> > backtrackIterators;
    bool isSolved;

    inline qvarCtxt() {
        isSolved = false;
    }

    inline typename Accessor::Node* solve(typename Accessor::Node* node) {
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
            backtrackIterators.push_back(std::make_pair(begin,end));
            totalArrity += Accessor::getArity(begin) - 1;
            node = Accessor::getNode(begin);
        }

        isSolved = true;
        return node;
    }

    inline typename Accessor::Node* nextSol() {
        int totalArrity = 0;
        totalArrity -= backtrackIterators.back().first->first.arity - 1;

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
        totalArrity += backtrackIterators.back().first->first.arity - 1;
        // Found a valid next, now we need to complete it to the right arrity
        typename Accessor::Node* currentNode =
                backtrackIterators.back().first->second;
        while (totalArrity) {
            typename Accessor::Iterator begin =
                    Accessor::getChildrenBegin(currentNode);
            typename Accessor::Iterator end =
                    Accessor::getChildrenEnd(currentNode);
            backtrackIterators.push_back(std::make_pair(begin,end));
            // Updating currentNode and arrity
            currentNode = Accessor::getNode(begin);
            totalArrity += Accessor::getArity(begin) - 1;
        }
        // We have selected a different solution
        return currentNode;
    }
};

}

#endif // _SEARCHCONTEXTTYPES_HPP

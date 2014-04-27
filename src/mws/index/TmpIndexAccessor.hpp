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
#ifndef _MWS_INDEX_TMPINDEXACCESSOR_HPP
#define _MWS_INDEX_TMPINDEXACCESSOR_HPP
/**
  * @brief  Temporary Index Accessor
  * @file   TmpIndexAccessor.hpp
  * @author Corneliu-Claudiu Prodescu <c.prodescu@jacobs-university.de>
  * @date   17 Apr 2014
  */

#include "common/utils/compiler_defs.h"
#include "mws/index/MwsIndexNode.hpp"
#include "mws/types/FormulaPath.hpp"

namespace mws {
namespace index {

struct TmpIndexAccessor {
    typedef MwsIndexNode Index;
    typedef MwsIndexNode Node;
    typedef MwsIndexNode::_MapType::iterator Iterator;

    static Node* getRootNode(Index* index) {
        return index;
    }

    static Iterator getChildrenBegin(Node* node) {
        return node->children.begin();
    }

    static Iterator getChildrenEnd(Node* node) {
        return node->children.end();
    }

    static encoded_token_t getToken(const Iterator& it) {
        return it->first;
    }

    static Arity getArity(const Iterator& it) {
        return it->first.arity;
    }

    static Node* getNode(Index* index, const Iterator& it) {
        UNUSED(index);
        return it->second;
    }

    static Node* getChild(Index* index, Node* node, encoded_token_t token) {
        UNUSED(index);
        auto it = node->children.find(token);
        if (it == node->children.end()) {
            return NULL;
        } else {
            return it->second;
        }
    }

    static types::FormulaId getFormulaId(Node* node) {
        return node->id;
    }

    static uint64_t getHitsCount(Node* node) {
        return node->solutions;
    }
};

}  // namespace index
}  // namespace mws

#endif  // _MWS_INDEX_TMPINDEXACCESSOR_HPP

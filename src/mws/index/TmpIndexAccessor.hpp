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
#include "common/utils/ContainerIterator.hpp"
#include "mws/index/TmpIndex.hpp"
#include "mws/types/FormulaPath.hpp"

namespace mws {
namespace index {

class TmpIndexAccessor {
    typedef TmpIndexNode::_MapType::const_iterator _Iterator;

 public:
    typedef const TmpIndex Index;
    typedef const TmpIndexNode Node;
    typedef common::utils::ContainerIterator<_Iterator> Iterator;

    static Node* getRootNode(Index* index) { return index->mRoot; }
    static Iterator getChildrenIterator(Node* node) {
        return Iterator(node->children.begin(), node->children.end());
    }
    static encoded_token_t getToken(const Iterator& it) {
        return it.get()->first;
    }
    static Arity getArity(const Iterator& it) { return it.get()->first.arity; }
    static Node* getNode(Index* index, const Iterator& it) {
        UNUSED(index);
        return it.get()->second;
    }
    static Node* getChild(Index* index, Node* node, encoded_token_t token) {
        UNUSED(index);
        auto it = node->children.find(token);
        if (it == node->children.end()) {
            return nullptr;
        } else {
            return it->second;
        }
    }
    static types::FormulaId getFormulaId(Node* node) {
        TmpLeafNode* leaf = (TmpLeafNode*)node;
        return leaf->id;
    }
    static uint64_t getHitsCount(Node* node) {
        TmpLeafNode* leaf = (TmpLeafNode*)node;
        return leaf->solutions;
    }
};

}  // namespace index
}  // namespace mws

#endif  // _MWS_INDEX_TMPINDEXACCESSOR_HPP

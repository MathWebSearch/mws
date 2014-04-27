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
#ifndef _MWS_INDEX_INDEXACCESSOR_HPP
#define _MWS_INDEX_INDEXACCESSOR_HPP
/**
  * @brief  Index Accessor
  * @file   IndexAccessor.hpp
  * @author Corneliu-Claudiu Prodescu <c.prodescu@jacobs-university.de>
  * @date   25 Apr 2014
  */

#include "mws/index/index.h"

namespace mws {
namespace index {

struct IndexAccessor {
    typedef index_handle_t Index;
    typedef inode_t Node;

public:
    class Iterator {
        inode_t* _node;
        int _index;

        Iterator(inode_t* node, int index)
            : _node(node), _index(index) {
        }
    public:
        Iterator& operator++(int) {
            _index++;
            return *this;
        }

        bool operator==(const Iterator& rhs) {
            return (_index == rhs._index) && (_node == rhs._node);
        }

        Iterator& operator=(const Iterator rhs) {
            if (this == &rhs) return *this;
            _node = rhs._node;
            _index = rhs._index;
            return *this;
        }

        friend struct IndexAccessor;
    };

    static Node* getRootNode(Index* index) {
        return index->root;
    }

    static Iterator getChildrenBegin(Node* node) {
        assert(node->type == INTERNAL_NODE);
        return Iterator(node, 0);
    }

    static Iterator getChildrenEnd(Node* node) {
        assert(node->type == INTERNAL_NODE);
        return Iterator(node, node->size);
    }

    static encoded_token_t getToken(const Iterator& it) {
        assert(it._node->type == INTERNAL_NODE);
        return it._node->data[it._index].token;
    }

    static Arity getArity(const Iterator& it) {
        return getToken(it).arity;
    }

    static Node* getNode(Index* index, const Iterator& it) {
        assert(it._node->type == INTERNAL_NODE);
        memsector_off_t off = it._node->data[it._index].off;
        assert(off != MEMSECTOR_OFF_NULL);
        return (Node*) memsector_off2addr(index->alloc, off);
    }

    static Node* getChild(Index* index, Node* node, encoded_token_t token) {
        assert(node->type == INTERNAL_NODE);
        memsector_off_t off = inode_get_child(node, token);
        if (off == MEMSECTOR_OFF_NULL) {
            return NULL;
        }
        return (Node*) memsector_off2addr(index->alloc, off);
    }

    static uint64_t getFormulaId(Node* node) {
        leaf_t* leaf = (leaf_t*) node;
        assert(leaf->type == LEAF_NODE);
        return leaf->formula_id;
    }

    static uint64_t getHitsCount(Node* node) {
        leaf_t* leaf = (leaf_t*) node;
        assert(leaf->type == LEAF_NODE);
        return leaf->num_hits;
    }
};

}  // namespace index
}  // namespace mws

#endif  // _MWS_INDEX_INDEXACCESSOR_HPP

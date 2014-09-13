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

#include "common/utils/ContainerIterator.hpp"
#include "mws/index/index.h"

namespace mws {
namespace index {

class IndexAccessor {
    struct _Iterator {
        typedef const inode_t Node;
        Node* _node;
        int _index;

        _Iterator(Node* node, int index) : _node(node), _index(index) {}
        _Iterator operator++(int) {
            _Iterator it = *this;
            _index++;
            return it;
        }
        bool operator==(const _Iterator& rhs) const {
            return (_index == rhs._index) && (_node == rhs._node);
        }
        bool operator!=(const _Iterator& rhs) const { return !(*this == rhs); }
    };

 private:
    typedef const inode_long_t LongNode;

 public:
    typedef const inode_t Node;
    typedef const index_handle_t Index;
    typedef common::utils::ContainerIterator<_Iterator> Iterator;

    static Node* getRootNode(Index* index) { return (Node*)index->root; }

    static Iterator getChildrenIterator(Node* node) {
        assert(node->type == INTERNAL_NODE || node->type == LONG_INTERNAL_NODE);
        return Iterator(_Iterator(node, 0), _Iterator(node, node->size));
    }
    static encoded_token_t getToken(const Iterator& it) {
        _Iterator _it = it.get();
        encoded_token_t tok;

        switch (_it._node->type) {
        case INTERNAL_NODE:
            tok = _it._node->data[_it._index].token;
            break;
        case LONG_INTERNAL_NODE:
            tok =
                reinterpret_cast<LongNode*>(_it._node)->data[_it._index].token;
            break;
        default:
            assert(_it._node->type == INTERNAL_NODE ||
                   _it._node->type == LONG_INTERNAL_NODE);
            break;
        }

        return tok;
    }
    static Arity getArity(const Iterator& it) { return getToken(it).arity; }
    static Node* getNode(Index* index, const Iterator& it) {
        UNUSED(index);
        _Iterator _it = it.get();
        memsector_long_off_t off;
        switch (_it._node->type) {
        case INTERNAL_NODE:
            off = _it._node->data[_it._index].off;
            break;
        case LONG_INTERNAL_NODE:
            off = reinterpret_cast<LongNode*>(_it._node)->data[_it._index].off;
            break;
        default:
            assert(_it._node->type == INTERNAL_NODE ||
                   _it._node->type == LONG_INTERNAL_NODE);
            break;
        }

        assert(off != MEMSECTOR_OFF_NULL);
        return (Node*)memsector_relOff2addr((char*)_it._node, off);
    }
    static Node* getChild(Index* index, Node* node, encoded_token_t token) {
        UNUSED(index);
        memsector_long_off_t off;
        switch (node->type) {
        case INTERNAL_NODE:
            off = inode_get_child(node, token);
            break;
        case LONG_INTERNAL_NODE:
            off =
                inode_long_get_child(reinterpret_cast<LongNode*>(node), token);
            break;
        default:
            assert(node->type == INTERNAL_NODE ||
                   node->type == LONG_INTERNAL_NODE);
            break;
        }

        if (off == MEMSECTOR_OFF_NULL) {
            return nullptr;
        }
        return (Node*)memsector_relOff2addr((char*)node, off);
    }
    static uint64_t getFormulaId(Node* node) {
        leaf_t* leaf = (leaf_t*)node;
        assert(leaf->type == LEAF_NODE);
        return leaf->formula_id;
    }
    static uint64_t getHitsCount(Node* node) {
        leaf_t* leaf = (leaf_t*)node;
        assert(leaf->type == LEAF_NODE);
        return leaf->num_hits;
    }
};

}  // namespace index
}  // namespace mws

#endif  // _MWS_INDEX_INDEXACCESSOR_HPP

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
  * @brief  Temporary Index implementation
  * @file   TmpIndex.cpp
  * @author Corneliu-Claudiu Prodescu <c.prodescu@jacobs-university.de>
  * @date   03 May 2011
  */

#include <string>
using std::string;
#include <stack>
using std::stack;
#include <queue>
using std::queue;
#include <vector>
using std::vector;
#include <utility>
using std::pair;
using std::make_pair;
#include <functional>
using std::function;

#include "mws/index/index.h"
#include "mws/index/TmpIndex.hpp"
#include "mws/index/TmpIndexAccessor.hpp"
#include "mws/index/CallbackIndexIterator.hpp"
#include "common/utils/compiler_defs.h"
#include "common/utils/ContainerIterator.hpp"
using common::utils::ContainerIterator;

// Static members declaration

namespace mws {
namespace index {

types::FormulaId TmpLeafNode::nextId = 0;

TmpIndexNode::TmpIndexNode() {}

TmpLeafNode::TmpLeafNode()
    : TmpIndexNode(), id(++TmpLeafNode::nextId), solutions(0) {}

TmpIndex::TmpIndex() : mRoot(new TmpIndexNode) {}

TmpIndex::~TmpIndex() {
    stack<TmpIndexNode*> nodes;
    nodes.push(mRoot);
    while (!nodes.empty()) {
        TmpIndexNode* node = nodes.top();
        nodes.pop();
        for (auto& kv : node->children) {
            nodes.push(kv.second);
        }
        delete node;
    }
}

TmpLeafNode* TmpIndex::insertData(
    const vector<encoded_token_t>& encodedFormula) {
    size_t size = encodedFormula.size();
    assert(size > 0);

    TmpIndexNode* currentNode = mRoot;
    for (size_t i = 0; i < size - 1; i++) {
        const encoded_token_t& encodedToken = encodedFormula[i];

        TmpIndexNode* node = (TmpIndexNode*)currentNode->children[encodedToken];
        if (node == nullptr) {
            currentNode->children[encodedToken] = node = new TmpIndexNode();
        }
        currentNode = node;
    }

    const encoded_token_t& encodedToken = encodedFormula[size - 1];
    TmpLeafNode* node = (TmpLeafNode*)currentNode->children[encodedToken];
    if (node == nullptr) {
        currentNode->children[encodedToken] = node = new TmpLeafNode();
    }

    return node;
}

memsector_long_off_t TmpIndex::_writeChildrenOffsets(
    memsector_writer_t* mswr, const TmpIndexNode* node,
    const vector<memsector_long_off_t>& offsets) {
    assert(node->children.size() == offsets.size());

    memsector_long_off_t currOffset =
            memsector_write_inode_begin(mswr, node->children.size(),
                                        offsets.front());

    int i = 0;
    for (const auto& entry : node->children) {
        memsector_write_inode_encoded_token_entry(mswr, entry.first,
                                                  currOffset - offsets[i]);
        i++;
    }
    memsector_write_inode_end(mswr);
    return currOffset;
}

uint64_t TmpIndex::computeMemsectorSize() const {
    uint64_t size = 0;

    auto onPush = [&](TmpIndexAccessor::Iterator iterator) {
        UNUSED(iterator);
    };
    auto onPop = [&](TmpIndexAccessor::Iterator iterator) {
        const TmpIndexNode* node = TmpIndexAccessor::getNode(this, iterator);
        uint32_t num_children = node->children.size();
        if (num_children > 0) {  // index node
            size += memsector_inode_size(num_children);
        } else {  // leaf node
            size += memsector_leaf_size();
        }
    };

    CallbackIndexIterator<TmpIndexAccessor> it(this, mRoot, onPush, onPop);
    // iterate through entire index, adding sizes of inodes and leafs
    while (it.next() != nullptr) continue;
    // add root size
    size += memsector_inode_size(mRoot->children.size());

    return size;
}

void TmpIndex::exportToMemsector(memsector_writer_t* mswr) const {
    stack<vector<memsector_long_off_t> > dfsStack;

    auto onPush = [&](TmpIndexAccessor::Iterator iterator) {
        const TmpIndexNode* node = TmpIndexAccessor::getNode(this, iterator);
        if (node->children.size() > 0) {
            dfsStack.push(vector<memsector_long_off_t>());
            dfsStack.top().reserve(node->children.size());
        }
    };

    auto onPop = [&](TmpIndexAccessor::Iterator iterator) {
        const TmpIndexNode* node = TmpIndexAccessor::getNode(this, iterator);
        if (node->children.size() > 0) {  // index node
            memsector_long_off_t offset =
                _writeChildrenOffsets(mswr, node, dfsStack.top());
            dfsStack.pop();
            dfsStack.top().push_back(offset);
        } else {  // leaf
            auto leaf = reinterpret_cast<const TmpLeafNode*>(node);
            memsector_long_off_t offset =
                    memsector_write_leaf(mswr, leaf->solutions, leaf->id);
            dfsStack.top().push_back(offset);
        }
    };

    dfsStack.push(vector<memsector_long_off_t>());
    CallbackIndexIterator<TmpIndexAccessor> it(this, mRoot, onPush, onPop);

    // iterate through entire index, writing inodes and leafs
    while (it.next() != nullptr) continue;
    // write the root, always use 64b offset for root
    memsector_long_off_t rootOffset =
        _writeChildrenOffsets(mswr, mRoot, dfsStack.top());

    dfsStack.pop();
    assert(dfsStack.empty());
    memsector_save(mswr, rootOffset);
}

}  // namespace index
}  // namespace mws

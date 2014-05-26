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

#include "mws/dbc/DbQueryManager.hpp"
#include "mws/index/index.h"
#include "mws/index/TmpIndex.hpp"
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

void TmpIndex::exportToMemsector(memsector_writer_t* mswr) const {
    struct NodeContext {
        const TmpIndexNode* node;
        ContainerIterator<TmpIndexNode::_MapType> childrenIterator;
        vector<memsector_off_t> childrenOffsets;

        explicit NodeContext(const TmpIndexNode* node)
            : node(node), childrenIterator(node->children) {
            childrenOffsets.reserve(node->children.size());
        }
    };
    stack<NodeContext> dfsStack;
    dfsStack.push(NodeContext(mRoot));

    memsector_off_t lastOffset = MEMSECTOR_OFF_NULL;
    while (!dfsStack.empty()) {
        if (dfsStack.top().childrenIterator.hasNext()) {
            auto it = dfsStack.top().childrenIterator.next();
            const TmpIndexNode* child = it->second;
            if (child->children.size() > 0) {  // internal node
                dfsStack.push(NodeContext(child));
            } else {  // leaf node
                const TmpLeafNode* leaf = (TmpLeafNode*)child;
                lastOffset =
                    memsector_write_leaf(mswr, leaf->solutions, leaf->id);
                dfsStack.top().childrenOffsets.push_back(lastOffset);
            }
        } else {
            const TmpIndexNode* node = dfsStack.top().node;
            lastOffset =
                memsector_write_inode_begin(mswr, node->children.size());
            int i = 0;
            for (const auto& entry : node->children) {
                memsector_write_inode_encoded_token_entry(
                    mswr, entry.first, dfsStack.top().childrenOffsets[i]);
                i++;
            }
            memsector_write_inode_end(mswr);
            dfsStack.pop();
            if (!dfsStack.empty()) {
                dfsStack.top().childrenOffsets.push_back(lastOffset);
            }
        }
    }

    memsector_save(mswr, lastOffset);
}

}  // namespace index
}  // namespace mws

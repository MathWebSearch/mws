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
#include <vector>
using std::vector;
#include <utility>
using std::pair;
using std::make_pair;

#include "mws/index/TmpIndex.hpp"
#include "mws/dbc/DbQueryManager.hpp"
#include "common/utils/compiler_defs.h"

// Static members declaration


namespace mws {
namespace index {

types::FormulaId TmpLeafNode::nextId = 0;

TmpIndexNode::TmpIndexNode() {
}

TmpLeafNode::TmpLeafNode()
    : TmpIndexNode(), id(++TmpLeafNode::nextId), solutions(0) {
}

TmpIndex::TmpIndex()
    : mRoot(new TmpIndexNode) {
}

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

TmpLeafNode*
TmpIndex::insertData(const vector<encoded_token_t>& encodedFormula) {
    size_t size = encodedFormula.size();
    assert(size > 0);

    TmpIndexNode* currentNode = mRoot;
    for (size_t i = 0; i < size - 1; i++) {
        const encoded_token_t& encodedToken = encodedFormula[i];

        TmpIndexNode* node = (TmpIndexNode*)
                currentNode->children[encodedToken];
        if (node == NULL) {
            currentNode->children[encodedToken] = node = new TmpIndexNode();
        }
        currentNode = node;
    }

    const encoded_token_t& encodedToken = encodedFormula[size - 1];
    TmpLeafNode* node = (TmpLeafNode*) currentNode->children[encodedToken];
    if (node == NULL) {
        currentNode->children[encodedToken] = node = new TmpLeafNode();
    }

    return node;
}

uint64_t
TmpIndex::getMemsectorSize() const {
    uint64_t size = 0;
    stack<const TmpIndexNode*> nodes;
    nodes.push(mRoot);

    while (!nodes.empty()) {
        const TmpIndexNode* node = nodes.top();
        nodes.pop();

        if (node->children.size() > 0) {
            size += inode_size(node->children.size());
            for (auto& kv : node->children) {
                const TmpIndexNode* child = (TmpIndexNode*) kv.second;
                nodes.push(child);
            }
        } else {
            size += leaf_size();
        }
    }

    return size;
}

memsector_off_t
TmpIndexNode::exportToMemsector(memsector_alloc_header_t* alloc) const {
    memsector_off_t off;

    if (children.size() > 0) {  // internal node
        off = inode_alloc(alloc, this->children.size());
        inode_t *inode = (inode_t*) memsector_off2addr(alloc, off);
        inode->type = INTERNAL_NODE;
        inode->size = this->children.size();

        int i = 0;
        for (auto& kv : this->children) {
            const TmpIndexNode* child = (TmpIndexNode*) kv.second;
            inode->data[i].token = kv.first;
            inode->data[i].off = child->exportToMemsector(alloc);

            i++;
        }
    } else {  // leaf node
        const TmpLeafNode* leafNode =
                reinterpret_cast<const TmpLeafNode*>(this);
        off = leaf_alloc(alloc);
        leaf_t *leaf = (leaf_t*) memsector_off2addr(alloc, off);
        leaf->type = LEAF_NODE;
        leaf->num_hits = leafNode->solutions;
        leaf->formula_id = leafNode->id;
    }

    return off;
}

void
TmpIndex::exportToMemsector(memsector_writer_t* mswr) const {
    mRoot->exportToMemsector(mswr_get_alloc(mswr));
}

}  // namespace index
}  // namespace mws

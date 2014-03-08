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
  * @brief  MwsIndexNode implementation
  * @file   MwsIndexNode.cpp
  * @author Corneliu-Claudiu Prodescu <c.prodescu@jacobs-university.de>
  * @date   03 May 2011
  */

#include <string>
#include <vector>

#include "MwsIndexNode.hpp"
#include "mws/dbc/DbQueryManager.hpp"
#include "mws/xmlparser/loadMwsHarvestFromFd.hpp"
#include "common/utils/DebugMacros.hpp"
#include "common/utils/compiler_defs.h"

// Namespaces

using namespace mws;
using namespace mws::types;
using namespace std;

// Static members declaration

unsigned long long MwsIndexNode::nextNodeId = 0;


namespace mws
{

MwsIndexNode::MwsIndexNode() :
    id          ( ++MwsIndexNode::nextNodeId ),
    solutions   ( 0 )
{ }


MwsIndexNode::~MwsIndexNode()
{
#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_IN;
#endif

    _MapType::iterator it;

    for (it = children.begin(); it != children.end(); it ++)
    {
        delete it->second;
    }

#ifdef TRACE_FUNC_CALLS
    LOG_TRACE_OUT;
#endif
}


MwsIndexNode*
MwsIndexNode::insertData(const vector<encoded_token_t>& encodedFormula) {
    MwsIndexNode* currentNode;
    currentNode = this;

    for (encoded_token_t encodedToken : encodedFormula) {
        NodeInfo nodeInfo = make_pair(encoded_token_get_id(encodedToken),
                                      encoded_token_get_arity(encodedToken));

        auto mapIt = currentNode->children.find(nodeInfo);
        // If no such node exists, we create it
        if (mapIt == currentNode->children.end()) {
            pair<_MapType::iterator, bool> ret =
                currentNode->children.insert(make_pair(nodeInfo,
                                                       new MwsIndexNode()));
            currentNode = ret.first->second;
        } else {
            currentNode = mapIt->second;
        }
    }

    return currentNode;
}


struct mws_data {
    MwsIndexNode* node;
    unsigned      arity;
};

memsector_off_t
MwsIndexNode::exportToMemsector(memsector_alloc_header_t* alloc) const {
    memsector_off_t off;

    if (children.size() > 0) {  // internal node
        off = inode_alloc(alloc, this->children.size());
        inode_t *inode = (inode_t*) memsector_off2addr(alloc, off);
        inode->type = INTERNAL_NODE;
        inode->size = this->children.size();

        int i = 0;
        for (auto& kv : this->children) {
            const MwsIndexNode* child = kv.second;
            inode->data[i].token =
                    encoded_token(kv.first.first, kv.first.second, 1);
            inode->data[i].off = child->exportToMemsector(alloc);

            i++;
        }
    } else {  // leaf node
        off = leaf_alloc(alloc);
        leaf_t *leaf = (leaf_t*) memsector_off2addr(alloc, off);
        leaf->type = LEAF_NODE;
        leaf->num_hits = this->solutions;
        leaf->formula_id = this->id;
    }

    return off;
}

void
MwsIndexNode::exportToMemsector(memsector_writer_t* mswr) const {
    (void) mswr_index_begin(mswr);
    memsector_alloc_header_t* alloc = mswr_get_alloc(mswr);
    this->exportToMemsector(alloc);
}

}

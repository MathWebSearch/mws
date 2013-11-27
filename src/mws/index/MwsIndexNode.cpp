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

#include <stack>
#include <string>

#include "MwsIndexNode.hpp"
#include "mws/dbc/PageDbConn.hpp"
#include "mws/xmlparser/loadMwsHarvestFromFd.hpp"
#include "common/utils/DebugMacros.hpp"
#include "common/utils/macro_func.h"

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
MwsIndexNode::insertData(const CmmlToken* expression,
                         MeaningDictionary* meaningDictionary) {
    stack<const CmmlToken*> processStack;
    MwsIndexNode*     currentNode;
    NodeInfo          currentNodeInfo;

    _MapType :: iterator mapIt;
    CmmlToken::PtrList::const_reverse_iterator rIt;

    // Initializing the iterative procedure
    processStack.push(expression);
    currentNode = this;

    while (!processStack.empty())
    {
        // Getting the next token and pushing it in the right node
        const CmmlToken* currentToken = processStack.top();
        processStack.pop();

        MeaningId meaningId = meaningDictionary->put(currentToken->getMeaning());
        currentNodeInfo = make_pair(meaningId,
                                    currentToken->getChildNodes().size());

        mapIt = currentNode->children.find(currentNodeInfo);
        // If no such node exists, we create it
        if (mapIt == currentNode->children.end())
        {
            pair<_MapType::iterator, bool> ret =
                currentNode->children.insert(make_pair(currentNodeInfo,
                                                       new MwsIndexNode()));
            currentNode = ret.first->second;
        }
        else
        {
            currentNode = mapIt->second;
        }

        // Reverse order because we want to retrieve from the stack properly
        for (rIt  = currentToken->getChildNodes().rbegin();
             rIt != currentToken->getChildNodes().rend();
             rIt ++)
        {
            processStack.push(*rIt);
        }
    }

    return currentNode;
}


struct mws_data {
    MwsIndexNode* node;
    unsigned      arity;
};

void
MwsIndexNode::exportToMemsector(memsector_writer_t* mswr) {
    memsector_alloc_header_t* alloc = mswr_get_alloc(mswr);

    (void) mswr_index_begin(mswr);

    typedef MwsIndexNode::_MapType::iterator iterator;

    // walking the MWS index in DFS order
    // using a stack to hold nodes and arities at the respective depth
    stack<pair<pair<iterator, iterator>,int> > nodes_stack;
    stack<pair<inode_t*, int> > ms_nodes_stack;

    MwsIndexNode::_MapType map;
    map.insert(make_pair(NodeInfo(0,1), this)); // TODO explain
    nodes_stack.push(make_pair(make_pair(map.begin(), map.end()), 1));

    while (!nodes_stack.empty()) {
        assert(nodes_stack.size() == ms_nodes_stack.size() + 1);

        iterator& top_curr = nodes_stack.top().first.first;
        iterator& top_end  = nodes_stack.top().first.second;
        int arity = nodes_stack.top().second;

        if (top_curr == top_end) {
            nodes_stack.pop();
            ms_nodes_stack.pop();
        } else {
            inode_t *parent_inode = NULL;
            int parent_slot;
            memsector_off_t off;
            int tok_arity = top_curr->first.second;
            int new_arity = arity + tok_arity - 1;

            // check if a parent inode exists
            if (!ms_nodes_stack.empty()) {
                parent_inode = ms_nodes_stack.top().first;
                parent_slot = ms_nodes_stack.top().second;
                ms_nodes_stack.top().second++;
            }

            if (new_arity == 0) {   // leaf node
                MwsIndexNode* leaf = top_curr->second;

                assert(new_arity == 0);

                // copy leaf node to memsector
                off = leaf_alloc(alloc);
                leaf_t *leaf_ms = (leaf_t*) memsector_off2addr(alloc, off);
                leaf_ms->type = LEAF_NODE;
                leaf_ms->num_hits = leaf->solutions;
                leaf_ms->dbid = leaf->id;

            } else {                // intermediary node
                MwsIndexNode* node = top_curr->second;

                assert(new_arity > 0);

                // copy index node to memsector
                off = inode_alloc(alloc, node->children.size());
                inode_t *inode = (inode_t*) memsector_off2addr(alloc, off);
                inode->type = INTERNAL_NODE;
                inode->size = node->children.size();
                
                // save in stacks
                ms_nodes_stack.push(make_pair(inode, 0));
                nodes_stack.push(make_pair(make_pair(node->children.begin(),
                                                     node->children.end()),
                                           new_arity));
            }

            // store link in parent
            if (parent_inode != NULL) {
                encoded_token_dict_entry_t entry;

                entry.token = encoded_token(top_curr->first.first,
                                            top_curr->first.second);
                entry.off = off;

                parent_inode->data[parent_slot] = entry;
            }

            top_curr++;
        }
    }
}

}

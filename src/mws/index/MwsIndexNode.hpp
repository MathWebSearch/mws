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
#ifndef _MWSINDEXNODE_HPP
#define _MWSINDEXNODE_HPP

/**
  * @brief  File containing the header of MwsIndexNode Class
  * @file   MwsIndexNode.hpp
  * @author Corneliu-Claudiu Prodescu <c.prodescu@jacobs-university.de>
  * @date   03 May 2011
  *
  * License: GPL v3
  *
  */

#include <stack>
#include <utility>
#include <vector>

#include "common/utils/util.hpp"
#include "mws/types/CmmlToken.hpp"
#include "mws/types/MwsAnswset.hpp"
#include "mws/types/VectorMap.hpp"
#include "mws/index/encoded_token.h"
#include "mws/index/memsector.h"
#include "mws/index/MeaningDictionary.hpp"

namespace mws {
namespace index {
struct TmpIndexAccessor;
}

class MwsIndexNode {
    typedef mws::VectorMap<encoded_token_t, MwsIndexNode*> _MapType;
    /// Map of children MwsIndex Nodes
    _MapType children;
    /// Id of the next node to be created
    static unsigned long long nextNodeId;
 public:
    /// Id of the MwsIndexNode
    const unsigned long long id;
    /// Number of solutions associated with this node
    unsigned int solutions;

    /**
      * @brief Default constructor of the MwsIndexSubst class
      */
    MwsIndexNode();

    /**
      * @brief Destructor of the MwsIndexSubst class
      */
    ~MwsIndexNode();

    /** @brief Method to insert data into the Index Tree
      * @param encodedFormula encoded formula
      * @return leaf node corresponding to the inserted expression
      */
    MwsIndexNode*
    insertData(const std::vector<encoded_token_t>& encodedFormula);

    /**
     * @brief exportToMemsector dump index data to a memsector index
     * @param mswr memsector writer handle
     */
    void exportToMemsector(memsector_writer_t* mswr) const;

 protected:
    memsector_off_t exportToMemsector(memsector_alloc_header_t* alloc) const;

    friend struct mws::index::TmpIndexAccessor;

    ALLOW_TESTER_ACCESS;
};

}

#endif // _MWSINDEXNODE_HPP

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
  * @brief  TmpIndex interfaces
  * @file   TmpIndex.hpp
  * @author Corneliu-Claudiu Prodescu <c.prodescu@jacobs-university.de>
  * @date   03 May 2011
  *
  * License: GPL v3
  *
  */

#include <stack>
#include <utility>
#include <vector>
#include <map>

#include "common/utils/util.hpp"
#include "mws/types/CmmlToken.hpp"
#include "mws/types/MwsAnswset.hpp"
#include "mws/types/VectorMap.hpp"
#include "mws/index/encoded_token.h"
#include "mws/index/memsector.h"
#include "mws/index/MeaningDictionary.hpp"
#include "mws/types/FormulaPath.hpp"

namespace mws {
namespace index {

struct TmpIndexAccessor;
class IndexBuilder;
class TmpIndex;

class TmpIndexNode {
    /*
    struct EncodedTokenLess {
        bool operator()(const encoded_token_t& t1,
                        const encoded_token_t& t2) const {
            return ((t1.arity < t2.arity) ||
                    (t1.arity == t2.arity && t1.id < t2.id));
        }
    };
    typedef std::map<encoded_token_t, TmpIndexNode*, EncodedTokenLess> _MapType;
    */
    typedef mws::VectorMap<encoded_token_t, TmpIndexNode*> _MapType;
    _MapType children;

 public:
    TmpIndexNode();

 private:
    friend struct TmpIndexAccessor;
    friend class TmpIndex;
    ALLOW_TESTER_ACCESS;
    DISALLOW_COPY_AND_ASSIGN(TmpIndexNode);
};

class TmpLeafNode : public TmpIndexNode {
    static types::FormulaId nextId;
    const types::FormulaId id;
    /// Number of solutions associated with this node
    unsigned int solutions;

 public:
    TmpLeafNode();

 private:
    friend struct mws::index::TmpIndexAccessor;
    friend class IndexBuilder;
    friend class TmpIndex;
    friend class TmpIndexNode;
    ALLOW_TESTER_ACCESS;
    DISALLOW_COPY_AND_ASSIGN(TmpLeafNode);
};

class TmpIndex {
    TmpIndexNode* mRoot;

 public:
    TmpIndex();
    ~TmpIndex();

    /** @brief Method to insert data into the Index Tree
      * @param encodedFormula encoded formula
      * @return leaf node corresponding to the inserted expression
      */
    TmpLeafNode*
    insertData(const std::vector<encoded_token_t>& encodedFormula);

    uint64_t getMemsectorSize() const;

    /**
     * @brief exportToMemsector dump index data to a memsector index
     * @param mswr memsector writer handle
     */
    void exportToMemsector(memsector_writer_t* mswr) const;

 private:
    friend struct TmpIndexAccessor;
    ALLOW_TESTER_ACCESS;
    DISALLOW_COPY_AND_ASSIGN(TmpIndex);
};

}  // namespace index
}  // namespace mws

#endif // _MWSINDEXNODE_HPP

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
#ifndef _SEARCHCONTEXT_HPP
#define _SEARCHCONTEXT_HPP

/**
  * @brief  File containing the header of SearchContext Class
  * @file   SearchContext.hpp
  * @author Corneliu-Claudiu Prodescu <c.prodescu@jacobs-university.de>
  * @date   25 May 2011
  *
  * License: GPL v3
  *
  */

#include <utility>
#include <map>
#include <vector>
#include <string>
#include <list>

#include "mws/dbc/DbQueryManager.hpp"
#include "mws/index/encoded_token.h"
#include "mws/types/CmmlToken.hpp"
#include "mws/types/MwsAnswset.hpp"
#include "mws/types/Query.hpp"

namespace mws { namespace query {

class SearchContext {
 public:
    /**
      * @param encodedFormula
      */
    SearchContext(const std::vector<encoded_token_t>& encodedFormula,
                  const types::Query::Options& options);

    /**
      * @brief Method to get the result of the search context, starting with
      * a given index node.
      * @param aNode is the index node where to start.
      * @param anOffset is the offset where to start returning the solutions.
      * @param aSize is the maximum number of solutions to return.
      * @param aMaxTotal is the maximum number of soulutions to count (with or
      * without returning).
      * @return an answer set with the corresponding results.
      */
    template<class Accessor>
    mws::MwsAnswset* getResult(typename Accessor::Index* aNode,
                               dbc::DbQueryManager* dbQueryManager,
                               unsigned int anOffset,
                               unsigned int aSize,
                               unsigned int aMaxTotal);

 private:
    struct _NodeTriple {
        bool           isQvar;
        MeaningId meaningId;
        Arity     arity;
        _NodeTriple(bool isQvar, MeaningId aMeaningId, Arity anArity);
    };

    int mQvarCount;
    /// Vector containing the DFS traversal of the query expression
    std::vector<_NodeTriple> expr;
    /// Qvar points in the Cmml Dfs Vector from where to backtrack. The
    /// vector starts with -1 to mark the beginning
    std::vector<int> backtrackPoints;
    types::Query::Options options;
};

}  // namespace query
}  // namespace mws

#endif // _SEARCHCONTEXT_HPP

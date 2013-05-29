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

// System includes

#include <utility>                     // STL pair
#include <map>
#include <vector>
#include <string>
#include <list>

// Local includes

#include "mws/types/CmmlToken.hpp"     // CmmlToken class header
#include "mws/types/MwsAnswset.hpp"    // MWS Answer set class header
#include "mws/dbc/PageDbConn.hpp"       // MWS Page Database connection class header
#include "mws/dbc/PageDbHandle.hpp"     // MWS Page Database handle class
#include "mws/index/MwsIndexNode.hpp"  // MWS Index node class header

#include "SearchContextTypes.hpp"

// Typedefs

namespace mws
{

struct SearchContext
{
    /// Vector containing the DFS traversal of the query expression
    std::vector<nodeTriple> expr;
    /// Table containing resolved Qvar and backtrack points
    std::vector<mws::qvarCtxt> qvarTable;
    /// Qvar points in the Cmml Dfs Vector from where to backtrack. The
    /// vector starts with -1 to mark the beginning
    std::vector<int> backtrackPoints;
    /// Qvar names
    std::vector<std::string> qvarNames;
    /// Qvar xpaths
    std::vector<std::string> qvarXpaths;

    // Constructors and Destructors

    /**
      * @brief Constructor of the SearchContext class.
      * @param expression is a pointer to a CmmlToken from which to create
      * the SearchContext instance.
      */
    SearchContext(mws::CmmlToken* expression, MeaningDictionary *dict);

    /**
      * @brief Destructor of the SearchContext class.
      */
    ~SearchContext();

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
    mws::MwsAnswset* getResult(mws::MwsIndexNode* aNode,
                               PageDbHandle* dbhandle,
                               unsigned int       anOffset,
                               unsigned int       aSize,
                               unsigned int       aMaxTotal);

};

}

#endif // _SEARCHCONTEXT_HPP

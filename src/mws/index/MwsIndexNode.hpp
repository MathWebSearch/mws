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

// System includes

#include <stack>                       // STL stack class header
#include <utility>                     // STL pair

// Local includes

#include "mws/dbc/PageDbConn.hpp"       // MWS Database connection class
#include "mws/types/CmmlToken.hpp"     // CmmlToken class header
#include "mws/types/MwsAnswset.hpp"    // MWS Answer set class header
#include "mws/types/NodeInfo.hpp"      // MWS node meaning declaration
#include "mws/types/MeaningDictionary.hpp" // MWS meaning dictionary
#include "mws/types/VectorMap.hpp"
#include "mws/index/memsector.h"


namespace mws
{

class MwsIndexNode
{
// Map to select children based on _MapKeyType
typedef mws::VectorMap<NodeInfo, MwsIndexNode*>                           _MapType;

private:
    /// Id of the next node to be created
    static unsigned long long nextNodeId;
    /// Id of the MwsIndexNode
    const unsigned long long id;
    /// Number of solutions associated with this node
    unsigned int solutions;
    /// Map of children MwsIndex Nodes
    _MapType children;

public:
    /**
      * @brief Default constructor of the MwsIndexSubst class
      */
    MwsIndexNode();

    /**
      * @brief Destructor of the MwsIndexSubst class
      */
    ~MwsIndexNode();

    /**
      * @brief Method to insert data into the Index Tree
      * @param expression is the CmmlToken to be indexed.
      * @param conn is the PageDbConn which will be used to insert the data
      * into the database
      * @param url is the address to be linked to the respective expression
      * @param xpath is the xpath to be linked to the respective expression
      * @return 1 if the expression was inserted and 0 otherwise.
      */
    int insertData(CmmlToken*  expression,
                   PageDbConn*  conn,
                   const char* url,
                   const char* xpath);

    void exportToMemsector(memsector_writer_t* mswr);
    // Friend declarations
    friend class SearchContext;
    friend class qvarCtxt;
    friend class Tester;
};

}

#endif // _MWSINDEXNODE_HPP

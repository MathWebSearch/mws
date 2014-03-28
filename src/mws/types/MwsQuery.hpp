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
#ifndef _MWSQUERY_HPP
#define _MWSQUERY_HPP

/**
  * @brief  File containing the header of the MwsQuery class.
  *
  * @file   MwsQuery.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date   19 Apr 2011
  *
  * License: GPL v3
  *
  */

// System includes

#include <cstdio>                      // C standard input output library
#include <vector>                      // STL vector headers

// Local includes

#include "mws/types/CmmlToken.hpp"     // Content MathML token class header
#include "mws/types/GenericTypes.hpp"  // MWS generic datatypes
#include "common/types/DataFormat.hpp"

#include "build-gen/config.h"

namespace mws
{

/**
  * @brief Datatype used to store a MWS Query
  *
  */
struct MwsQuery
{
    /// Variable used to show the number of warnings (-1 for critical error)
    int                          warnings;
    /// Vector containing pointers to the CMML tokens which have been read
    std::vector<types::CmmlToken*> tokens;
    /// Value showing the maximum number of results to be returned
    size_t                       attrResultMaxSize;
    /// Value showing the index from which to return results
    size_t                       attrResultLimitMin;
    /// BoolValue showing if the total count is required
    bool                         attrResultTotalReq;
    /// Value showing the maxmum number of result to be counted (returned or
    /// not)
    int                          attrResultTotalReqNr;
    /// Format of the output (xml, json, etc)
    DataFormat                   attrResultOutputFormat;
    /// Boolean value showing if the query needed restrictions
    bool                         restricted;
    
    /// Default Constructor of the MwsQuery class
    MwsQuery() :
        warnings(0),
        attrResultMaxSize(DEFAULT_QUERY_RESULT_SIZE),
        attrResultLimitMin(0),
        attrResultTotalReq(DEFAULT_QUERY_TOTALREQ),
        attrResultTotalReqNr(DEFAULT_QUERY_RESULT_TOTAL),
        attrResultOutputFormat(DATAFORMAT_DEFAULT),
        restricted(false) {
    }

    /// Destructor of the MwsQuery class
    ~MwsQuery() {
        std::vector<types::CmmlToken*> :: iterator it;
        for (it = tokens.begin(); it != tokens.end(); it++)
            delete (*it);
    }

    void applyRestrictions()
    {
        if (attrResultMaxSize > MAX_QUERY_RESULT_SIZE)
        {
            restricted = true;
            attrResultMaxSize = MAX_QUERY_RESULT_SIZE;
        }
        if (attrResultLimitMin > MAX_QUERY_OFFSET)
        {
            restricted = true;
            attrResultLimitMin = MAX_QUERY_OFFSET;
        }
        if (attrResultTotalReqNr > MAX_QUERY_RESULT_TOTAL)
        {
            restricted = true;
            attrResultTotalReqNr = MAX_QUERY_RESULT_TOTAL;
        }
    }

    /// Service method for printing the contents of a MwsQuery
    void print()
    {
        std::vector<types::CmmlToken*> :: iterator it;
        printf("MwsQuery details:\n");
        printf("Warnings: %d\n", warnings);
        printf("Expressions (%d):\n", (int) tokens.size());
        //for (it = tokens.begin(); it != tokens.end(); it++)
        //    (*it)->print();
        printf("MaxSize : %d\n", (int) attrResultMaxSize);
        printf("LimitMin: %d\n", (int) attrResultLimitMin);
    }
};

}

#endif // _MWSQUERY_HPP

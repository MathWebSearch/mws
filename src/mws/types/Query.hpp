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
#ifndef _MWS_TYPES_QUERY_HPP
#define _MWS_TYPES_QUERY_HPP

/**
  * @brief  MWS Query
  *
  * @file   Query.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date   19 Apr 2011
  */

#include <cstdio>
#include <string>
#include <vector>

#include "common/utils/compiler_defs.h"
#include "mws/types/CmmlToken.hpp"
#include "mws/types/GenericAnswer.hpp"

#include "build-gen/config.h"

namespace mws {
namespace types {

/**
  * @brief MathWebSearch Query
  */
struct Query {
    struct ResponseFormatter {
        virtual const char* getContentType() const = 0;
        virtual int writeData(const GenericAnswer* ans, FILE* output) const = 0;
        ResponseFormatter() {}
        DISALLOW_COPY_AND_ASSIGN(ResponseFormatter);
    };

    struct Options {
        bool includeHits;
        bool includeMwsIds;

        Options() : includeHits(true), includeMwsIds(true) {}
    };

    /// Variable used to show the number of warnings (-1 for critical error)
    int warnings;
    /// Vector containing pointers to the CMML tokens which have been read
    std::vector<types::CmmlToken*> tokens;
    /// Value showing the maximum number of results to be returned
    size_t attrResultMaxSize;
    /// Value showing the index from which to return results
    size_t attrResultLimitMin;
    /// BoolValue showing if the total count is required
    bool attrResultTotalReq;
    /// Value showing the maxmum number of result to be counted (returned or
    /// not)
    int attrResultTotalReqNr;
    const ResponseFormatter* responseFormatter;
    Options options;
    /// Boolean value showing if the query needed restrictions
    bool restricted;
    /// Depth used for schema-queries; ignored for regular mws-queries
    uint8_t max_depth;

    /// Default Constructor of the MwsQuery class
    Query()
        : warnings(0),
          attrResultMaxSize(DEFAULT_QUERY_RESULT_SIZE),
          attrResultLimitMin(0),
          attrResultTotalReq(DEFAULT_QUERY_TOTALREQ),
          attrResultTotalReqNr(DEFAULT_QUERY_RESULT_TOTAL),
          restricted(false),
          max_depth(DEFAULT_SCHEMA_DEPTH) {}

    /// Destructor of the MwsQuery class
    ~Query() {
        std::vector<types::CmmlToken*>::iterator it;
        for (it = tokens.begin(); it != tokens.end(); it++) delete (*it);
    }

    void applyRestrictions() {
        if (attrResultMaxSize > MAX_QUERY_RESULT_SIZE) {
            restricted = true;
            attrResultMaxSize = MAX_QUERY_RESULT_SIZE;
        }
        if (attrResultLimitMin > MAX_QUERY_OFFSET) {
            restricted = true;
            attrResultLimitMin = MAX_QUERY_OFFSET;
        }
        if (attrResultTotalReqNr > MAX_QUERY_RESULT_TOTAL) {
            restricted = true;
            attrResultTotalReqNr = MAX_QUERY_RESULT_TOTAL;
        }
    }
};

}  // namespace types
}  // namespace mws

#endif  // _MWS_TYPES_QUERY_HPP

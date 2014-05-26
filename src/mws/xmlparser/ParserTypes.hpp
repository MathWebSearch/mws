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
#ifndef _PARSERTYPES_HPP
#define _PARSERTYPES_HPP

/**
  * @brief File containing the headers of the MWS SAX Parser user data
  *
  * @file ParserTypes.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date 19 Apr 2011
  *
  * License: GPL v3
  *
  */

#include <stdio.h>

#include <stack>
#include <string>
#include <vector>

#include "mws/types/CmmlToken.hpp"
#include "mws/types/MwsQuery.hpp"
#include "mws/index/IndexBuilder.hpp"

namespace mws
{

/**
  * @brief Enumeration holding the states of the Mws Query Sax Parser.
  */
enum MwsQueryState
{
    MWSQUERYSTATE_DEFAULT,
    MWSQUERYSTATE_IN_MWS_QUERY,
    MWSQUERYSTATE_IN_MWS_EXPR,
    MWSQUERYSTATE_UNKNOWN,
};

/**
  * @brief Data type used to store the variables needed during the parsing of
  * a MwsQuery
  */
struct MwsQuery_SaxUserData
{
    /// Depth of the parse tree in unknown state
    int                          unknownDepth;
    /// The token which is currently being parsed
    types::CmmlToken*            currentToken;
    /// The root of the token being currently parsed
    types::CmmlToken*              currentTokenRoot;
    /// State of the parsing
    mws::MwsQueryState           state;
    /// State of the parsing before going into an unknown state
    mws::MwsQueryState           prevState;
    /// True if an XML structural error is detected
    bool                         errorDetected;
    /// Result of the parsing
    mws::MwsQuery*               result;

    MwsQuery_SaxUserData()
    {
        currentToken = nullptr;
        currentTokenRoot = nullptr;
        result = nullptr;
    }
};

struct MwsMessage_SaxUserData
{
    // True is we found the nature of the message, False if not
    int                           messageTypeFound;
    // string describing the first tag of the MwsMessage
    std::string                   messageType;
};

}

#endif // _PARSERTYPES_HPP

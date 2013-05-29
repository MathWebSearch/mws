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

// System includes

#include <cstdio>                      // C standard input output library
#include <string>                      // C++ String header
#include <stack>                       // C++ Stack header

// Local includes

#include "mws/types/CmmlToken.hpp"     // Content MathML token class header
#include "mws/types/MwsQuery.hpp"      // Mws Query class header
#include "mws/index/MwsIndexNode.hpp"  // Mws Index Node class header
#include "mws/dbc/PageDbHandle.hpp"    // Page DbHandle

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
    mws::CmmlToken*              currentToken;
    /// The root of the token being currently parsed
    mws::CmmlToken*              currentTokenRoot;
    /// The stack containing the parent CMML Tokens
    std::stack<mws::CmmlToken*>  processStack;
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
        currentToken = NULL;
        currentTokenRoot = NULL;
        result = NULL;
    }
};

/**
  * @brief Enumeration holding the states of the Mws Harvest Sax Parser.
  */
enum MwsHarvestState
{
    MWSHARVESTSTATE_DEFAULT,
    MWSHARVESTSTATE_IN_MWS_HARVEST,
    MWSHARVESTSTATE_IN_MWS_EXPR,
    MWSHARVESTSTATE_UNKNOWN
};

/**
  * @brief Data type used to store the variables needed during the parsing of
  * a MwsQuery
  */
struct MwsHarvest_SaxUserData
{
    /// Depth of the parse tree in unknown state
    int                          unknownDepth;
    /// The token which is currently being parsed
    mws::CmmlToken*              currentToken;
    /// The root of the token being currently parsed
    mws::CmmlToken*              currentTokenRoot;
    /// The stack containing the parent CMML Tokens
    std::stack<mws::CmmlToken*>  processStack;
    /// State of the parsing
    mws::MwsHarvestState         state;
    /// State of the parsing before going into an unknown state
    mws::MwsHarvestState         prevState;
    /// True if an XML structural error is detected
    bool                         errorDetected;
    /// Number of correctly parsed expressions
    int                          parsedExpr;
    /// Variable used to show the number of warnings (-1 for critical error)
    int                          warnings;
    /// URI of the expression being parsed
    std::string                  exprUri;
    /// MwsIndexNode where to add expressions from the harvest
    mws::MwsIndexNode*           indexNode;
    /// PageDbHandle pointer
    PageDbHandle*                dbhandle;
    /// Meaning dictionary
    MeaningDictionary*           meaningDict;
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

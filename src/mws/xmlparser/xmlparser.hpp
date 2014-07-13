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
#ifndef _MWS_PARSER_XMLPARSER_HPP
#define _MWS_PARSER_XMLPARSER_HPP

/**
  * @file xmlparser.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date 05 May 2011
  *
  */

namespace mws {
namespace parser {
    
/**
  * @brief Method to initialize the xmlparser module of Math Web Search.
  * @return 0 if the initialization succeeds with multi-threading support
  * @return 1 if the initialization succeeds w/o multi-threading support
  * @return -1 on failure
  */
int initxmlparser();

/**
 * @brief release xmlparser resources
 */
void clearxmlparser();

}  // namespace parser
}  // namespace mws

#endif  // _MWS_PARSER_XMLPARSER_HPP

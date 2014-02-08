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
/**
 * Function to extract MathML formulas from a website
 * coded in HTML or XHTML
 * @file MathParser.hpp
 *
 * date: 15 Aug 2012
 */
#ifndef _CRAWLER_PARSER_MATHPARSER_HPP
#define _CRAWLER_PARSER_MATHPARSER_HPP

#include <string>
#include <vector>



namespace crawler { namespace parser {

/**
 * @param xhtml XHTML content
 * @param url URL of the XHTML content to be attributed to the harvested
 * expressions
 * @return vector of harvested expressions
 */
std::vector<std::string> getHarvestFromXhtml(const std::string& xhtml,
                                             const int data_id);

} }


#endif // _CRAWLER_PARSER_MATHPARSER_HPP

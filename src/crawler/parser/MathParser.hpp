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
 *
 * @file MathParser.hpp
 * @date 15 Aug 2012
 */
#ifndef _CRAWLER_PARSER_MATHPARSER_HPP
#define _CRAWLER_PARSER_MATHPARSER_HPP

#include <stdexcept>
#include <string>
#include <vector>

namespace crawler {
namespace parser {

struct Harvest {
    std::string dataElement;
    std::vector<std::string> contentMathElements;
};

/**
 * @param content HTML or XHTML content
 * @param url URL of the HTML or XHTML
 * @param data_id
 *
 * @return vector of harvest data and expressions
 *
 * @throw runtime_error with the message
 */
Harvest getHarvestFromDocument(const std::string& content,
                               const int data_id,
                               const std::string& title_xpath,
                               bool save_data)
throw (std::runtime_error);

/**
 * @brief cleanupMathParser should be called at the end of the process to
 * release all libxml global resources
 */
void cleanupMathParser();

}  // namespace parser
}  // namespace crawler


#endif // _CRAWLER_PARSER_MATHPARSER_HPP

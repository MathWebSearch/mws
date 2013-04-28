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
 * authors: Daniel Hasegan & Catalin Perticas
 * date: 15 Aug 2012
 */
#ifndef _MWSGETMATH_HPP
#define _MWSGETMATH_HPP

// System Includes
#include <string>
#include <vector>
#include <set>

// Local includes
#include "crawler/types/Page.hpp"

// Global variable
extern std::set < std::string > mathml_tags;

namespace mws {
// HTML
int isHTML(Page& page);
std::vector< std::string > get_math_html(Page& page);
void store_mathml_tags();

// XHTML
int isXHTML(Page& page);
std::vector< std::string > get_math_xhtml(const std::string& content, const std::string& url);

// GENERAL
std::vector< std::string > get_math(Page& page);
int is_good_xml(std::string xml);

} // namespace mws


#endif

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
#ifndef _COMMON_UTILS_GETBOOLTYPE_HPP
#define _COMMON_UTILS_GETBOOLTYPE_HPP

/**
  * @brief File containing the header of getBoolType utility function
  *
  * @file getBoolType.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date 22 Jun 2011
  *
  * License: GPL v3
  */

#include <string>                      // C string header

namespace common {
namespace utils {

enum BoolType {
    BOOL_DEFAULT,
    BOOL_YES,
    BOOL_NO,
};

/**
  * @brief Method to parse a char array and get the boolean value.
  * @param str is a char array containing a bool value ("1", "0", "yes", etc)
  * @return a BoolType value corresponding to the input.
  */
BoolType getBoolType(std::string);

}  // namespace utils
}  // namespace common

#endif  // _COMMON_UTILS_GETBOOLTYPE_HPP

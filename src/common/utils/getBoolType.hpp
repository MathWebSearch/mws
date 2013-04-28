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
#ifndef _GETBOOLTYPE_HPP
#define _GETBOOLTYPE_HPP

/**
  * @brief File containing the header of getBoolType utility function
  *
  * @file getBoolType.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date 22 Jun 2011
  *
  * License: GPL v3
  */

// System includes

#include <string>                      // C string header

// Local includes

#include "mws/types/GenericTypes.hpp"  // MWS generic datatypes


/**
  * @brief Method to parse a char array and get the boolean value.
  * @param str is a char array containing a bool value ("1", "0", "yes", etc)
  * @return a BoolType value corresponding to the input.
  */
mws::BoolType
getBoolType(std::string);

#endif // _GETBOOLTYPE_HPP

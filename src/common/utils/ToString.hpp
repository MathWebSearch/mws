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
#ifndef _TOSTRING_HPP
#define _TOSTRING_HPP

/**
  * @brief File containing the toString utility function
  *
  * @file ToString.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date 16 Jun 2011
  *
  * License: GPL v3
  */

// System includes

#include <sstream>                     // C++ string stream header

/**
  * Note that this method uses the stream insertion operator. This is defined
  * on the native datatypes and many others.
  * @brief Method to convert an item to string.
  * @param item is the item to be converted.
  * @return is a string equivalent to the item.
  */
template <class T>
inline std::string
ToString(const T item)
{
    std::stringstream s;

    s << item;
    return s.str();
}

#endif // _TOSTRING_HPP

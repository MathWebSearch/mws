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
  * @brief File containing the implementation of getBoolType utility function
  *
  * @file getBoolType.cpp
  * @author Corneliu-Claudiu Prodescu
  * @date 22 Jun 2011
  *
  * License: GPL v3
  */

// System includes

#include <string>  // C string header

// Local includes

#include "common/utils/getBoolType.hpp"  // GetBoolType function declaration

// Namespaces
using namespace std;
using namespace mws;

BoolType getBoolType(string str) {
    BoolType result;
    int i;
    const pair<string, BoolType> values[] = {
        make_pair("yes", BOOL_YES),  make_pair("1", BOOL_YES),
        make_pair("no", BOOL_NO),    make_pair("0", BOOL_NO),
        make_pair("", BOOL_DEFAULT),  // Flag to end array
    };

    result = BOOL_DEFAULT;

    // Lower-casing the buffer
    for (i = 0; str[i] != '\0'; i++) {
        str[i] = tolower(str[i]);
    }

    // Iterating through values
    for (i = 0; values[i].second != BOOL_DEFAULT; i++) {
        if (str == values[i].first) {
            result = values[i].second;
            break;
        }
    }

    return result;
}

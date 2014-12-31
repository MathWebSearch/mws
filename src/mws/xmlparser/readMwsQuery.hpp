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
#ifndef _READMWSQUERY_HPP
#define _READMWSQUERY_HPP

/**
  * @brief File containing the header of the readMwsQuery function
  *
  * @file readMwsQuery.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date 18 Apr 2011
  *
  * @edited Radu Hambasan
  * @date 20 Mar 2014
  *
  * License: GPL v3
  *
  */

#include <stdio.h>
#include "mws/types/Query.hpp"

namespace mws {
namespace xmlparser {

enum QueryMode {
    QUERY_MWS,
    QUERY_SCHEMA
};

/**
  * @brief Function to read a MwsQuery from an input file descriptor.
  * @param file is the file from where to read.
  * @return a pointer to a MwsQuery containing the information read or NULL in
  * case of failure.
  */
mws::types::Query* readMwsQuery(FILE* file, QueryMode mode = QUERY_MWS);

}  // namespace xmlparser
}  // namespace mws

#endif  // _READMWSQUERY

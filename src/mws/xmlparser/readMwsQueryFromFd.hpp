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
#ifndef _READMWSQUERYFROMFD_HPP
#define _READMWSQUERYFROMFD_HPP

/**
  * @brief File containing the header of the readMwsQueryFromFd function
  *
  * @file readMwsQueryFromFd.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date 18 Apr 2011
  *
  * License: GPL v3
  *
  */

// Local includes

#include "mws/types/MwsQuery.hpp"      // MwsQuery datatype header

namespace mws
{

/**
  * @brief Function to read a MwsQuery from an input file descriptor.
  * @param fd is the file descriptor from where to read.
  * @return a pointer to a MwsQuery containing the information read or NULL in
  * case of failure.
  */
mws::MwsQuery* readMwsQueryFromFd(int fd);

}

#endif

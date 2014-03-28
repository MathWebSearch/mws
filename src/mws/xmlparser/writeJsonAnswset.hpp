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
#ifndef _WRITEJSONANSWSET_HPP
#define _WRITEJSONANSWSET_HPP

/**
  * @brief   File containing the header of the writeJsonAnswset function
  *
  * @file    writeJsonAnswset.hpp
  * @author  Corneliu-Claudiu Prodescu
  * @date    30 Jul 2011
  *
  * @edited Radu Hambasan
  * @date 20 Mar 2014
  * License: GPL v3
  *
  */

#include <stdio.h>
// Local includes

#include "mws/types/MwsAnswset.hpp"    // MWS Answer Set datatype header

namespace mws
{

/**
  * @brief Function to write a MwsAnswset to an output file descriptor as JSON
  * data.
  * @param file is the file to which to write.
  * @param answset is the MWS Answer Set to be written.
  * @return the number of bytes written to file or -1 in case of failure.
  */
int writeJsonAnswset(mws::MwsAnswset* answset, FILE* file);

}

#endif  // _WRITEJSONANSWSET_HPP

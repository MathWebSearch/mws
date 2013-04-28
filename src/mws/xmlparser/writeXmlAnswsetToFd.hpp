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
#ifndef _WRITEXMLANSWSETTOFD_HPP
#define _WRITEXMLANSWSETTOFD_HPP

/**
  * @brief   File containing the header of the writeXmlAnswsetToFd function
  *
  * @file    writeXmlAnswsetToFd.hpp
  * @author  Corneliu-Claudiu Prodescu
  * @date    26 Apr 2011
  *
  * License: GPL v3
  *
  */

// Local includes

#include "mws/types/MwsAnswset.hpp"    // MWS Answer Set datatype header

namespace mws
{

/**
  * @brief Function to write a MwsAnswset to an output file descriptor.
  * @param fd is the file descriptor to which to write.
  * @param answset is the MWS Answer Set to be written.
  * @return the number of bytes written to fd or -1 in case of failure.
  */
int writeXmlAnswsetToFd(mws::MwsAnswset* answset, int fd);

}

#endif

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
#ifndef _VERIFYMWSMESSAGETYPEFROMFD_HPP
#define _VERIFYMWSMESSAGETYPEFROMFD_HPP

/**
  * @brief File containing the header of the verifyMwsMessageTypeFromFd function
  *
  * @file verifyMwsMessageTypeFromFd.hpp
  * @author Daniel Hasegan
  * @date 08 Aug 2012
  *
  * License: GPL v3
  *
  */

// System includes

#include <string>

// Local includes

namespace mws
{

/**
  * @brief Function to read a MwsMessage from an input file descriptor.
  * @param fd is the file descriptor from where to read.
  * @return a string describing the first tag of the MwsMessage
  */
std::string verifyMwsMessageTypeFromFd(int fd);

}

#endif

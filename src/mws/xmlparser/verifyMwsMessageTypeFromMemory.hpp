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
#ifndef _VERIFYMWSMESSAGETYPEFROMMEMORY_HPP
#define _VERIFYMWSMESSAGETYPEFROMMEMORY_HPP

/**
  * @brief File containing the header of the verifyMwsMessageTypeFromMemory function
  *
  * @file verifyMwsMessageTypeFromMemory.hpp
  * @author Daniel Hasegan
  * @date 13 Aug 2012
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
  * @brief Function to read a MwsMessage from a (char *) in memory
  * @param char* which is the whole message.
  * @return a string describing the first tag of the MwsMessage
  */
std::string verifyMwsMessageTypeFromMemory(char * mem);

}

#endif

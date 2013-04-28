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
#ifndef _LOADMWSHARVESTFROMMEMORY_HPP
#define _LOADMWSHARVESTFROMMEMORY_HPP

/**
  * @brief File containing the header of the loadMwsHarvestFromMemory function
  *
  * @file loadMwsHarvestFromMemory.hpp
  * @author Daniel Hasegan
  * @date 10 Aug 2012
  *
  * License: GPL v3
  *
  */

// System includes

#include <utility>                     // C++ utility headers (pair)

// Local includes

#include "mws/index/MwsIndexNode.hpp"  // MwsIndexNode datatype header

namespace mws
{

/**
  * @brief Function to load a MwsHarvest in from memory.
  * @param indexNode is a pointer to the MwsIndexNode where to load.
  * @param mem is the (char*) with the XML code to load.
  * @return a pair with an exit code (0 on success and -1 on failure) and
  * the number of successfully loaded entries.
  */
std::pair<int, int>
loadMwsHarvestFromMemory(mws::MwsIndexNode* indexNode, char* mem, PageDbHandle *dbhandle);

}

#endif

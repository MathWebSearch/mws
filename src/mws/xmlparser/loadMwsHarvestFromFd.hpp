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
#ifndef _MWS_PARSER_LOADMWSHARVESTFROMFD_HPP
#define _MWS_PARSER_LOADMWSHARVESTFROMFD_HPP

/**
  * @brief File containing the header of the loadMwsHarvestFromFd function
  *
  * @file loadMwsHarvestFromFd.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date 30 Apr 2011
  *
  * License: GPL v3
  *
  */

// System includes

#include <string>
#include <utility>

// Local includes

#include "mws/index/IndexManager.hpp"
#include "common/utils/Path.hpp"

namespace mws {
namespace parser {

/** @brief Function to load a MwsHarvest in from a file descriptor.
  * @param indexManager
  * @param fd is the file descriptor from where to read.
  * @return a pair with an exit code (0 on success and -1 on failure) and
  * the number of successfully loaded entries.
  */
std::pair<int, int>
loadMwsHarvestFromFd(index::IndexManager* indexManager, int fd);


int loadMwsHarvestFromDirectory(mws::index::IndexManager* indexManager,
                                const mws::AbsPath& dirPath,
                                const std::string& extension,
                                bool recursive);

}  // namespace parser
}  // namespace mws

#endif  // _MWS_PARSER_LOADMWSHARVESTFROMFD_HPP

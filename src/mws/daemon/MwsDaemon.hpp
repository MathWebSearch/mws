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
#ifndef _MWSDAEMON_HPP
#define _MWSDAEMON_HPP

/**
  * @brief File containing the header of the MwsDaemon class.
  * @file MwsDaemon.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date 18 Jun 2011
  *
  * License: GPL v3
  *
  */

#include <string>
#include <vector>
#include <inttypes.h>


// TODO Doc and clean up implementation

namespace mws { namespace daemon {

struct Config {
    std::vector<std::string> harvestLoadPaths;
    bool                     recursive;
    uint16_t                 mwsPort;
    std::string              dataPath;
};

int mwsDaemonLoop(const Config& config);

}}

#endif // _MWSDAEMON_HPP

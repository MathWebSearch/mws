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
#ifndef _MWSDAEMONCONF_HPP
#define _MWSDAEMONCONF_HPP

/**
  * @brief  File containing configuration macros for the MWS daemon module
  * @file   MwsDaemonConf.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date   20 May 2011
  *
  */

// Local includes

#include "common/utils/macro_func.h"

// Port where to run MWS
#define DEFAULT_MWS_PORT                26285

// Host where to run MWS
#define DEFAULT_MWS_HOST                "localhost"

// Data path (db files, index, etc)
#define DEFAULT_MWS_DATA_PATH           "/tmp"

// Extension of Mws Harvest files
#define DEFAULT_MWS_HARVEST_EXT         "xml"

#endif

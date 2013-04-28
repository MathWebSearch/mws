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
#ifndef _MWSDBCMACROS_HPP_
#define _MWSDBCMACROS_HPP_

/**
  * @file MwsDbcMacros.hpp
  * @brief File containing the MWS Database macro handling
  * @author Corneliu-Claudiu Prodescu
  * @date 03 Feb 2011
  *
  */

// Config files

#include "MwsDbcConf.hpp"// Dbc configuration header

// Macros handling

#ifndef MWS_DBENV_DIR
#define MWS_DBENV_DIR DEFAULT_MWS_DBENV_DIR
#endif

#ifndef MWS_DB_FILE
#define MWS_DB_FILE DEFAULT_MWS_DB_FILE
#endif

#ifndef MWS_DB_NAME
#define MWS_DB_NAME DEFAULT_MWS_DB_NAME
#endif

#ifndef MWS_DB_CACHESIZE
#define MWS_DB_CACHESIZE DEFAULT_MWS_DB_CACHESIZE
#endif

#ifndef MWS_DB_MMAPSIZE
#define MWS_DB_MMAPSIZE DEFAULT_MWS_DB_MMAPSIZE
#endif


#endif

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
#ifndef _MWSDBCCONF_HPP
#define _MWSDBCCONF_HPP

/**
  * @brief  File containing configuration macros for the MWS dbc module
  * @file   MwsDbcConf.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date   18 May 2011
  *
  */

// Directory where the database environment is stored
#define DEFAULT_MWS_DBENV_DIR          "/tmp"

// File where the database is stored (if relative, root is DBENV_DIR)
#define DEFAULT_MWS_DB_FILE            "mws.db"

// Logical database name
#define DEFAULT_MWS_DB_NAME            "mwsdb"

// Allocated cache size (bytes)
#define DEFAULT_MWS_DB_CACHESIZE       300 * 1024 * 1024

// Maximum mmap size per database file (bytes)
#define DEFAULT_MWS_DB_MMAPSIZE        300 * 1024 * 1024

#endif

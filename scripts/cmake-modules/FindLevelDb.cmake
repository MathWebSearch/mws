#
# Copyright (C) 2010-2013 KWARC Group <kwarc.info>
#
# This file is part of MathWebSearch.
#
# MathWebSearch is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# MathWebSearch is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with MathWebSearch.  If not, see <http://www.gnu.org/licenses/>.
#
#
# LEVELDB_FOUND - system has LevelDB
# LEVELDB_INCLUDE_DIR - the LevelDB include directory
# LEVELDB_LIBRARIES - Link these to use LevelDB
# LEVELDB_DEFINITIONS - Compiler switches required for using LevelDB
# LEVELDB_NEED_PREFIX - this is set if the functions are prefixed with LevelDB

IF (LEVELDB_INCLUDE_DIR AND LEVELDB_LIBRARIES)
    SET(LEVELDB_FIND_QUIETLY TRUE)
ENDIF (LEVELDB_INCLUDE_DIR AND LEVELDB_LIBRARIES)

FIND_PATH(LEVELDB_INCLUDE_DIR NAMES leveldb/db.h HINTS
   /usr/include
   /usr/local/include
   $ENV{LEVELDB}
   $ENV{LEVELDB}/include
   )

FIND_LIBRARY(LEVELDB_LIBRARIES NAMES leveldb leveldb.dll.a leveldb.a HINTS
   /usr/lib
   /usr/local/lib
   $ENV{LEVELDB}
   $ENV{LEVELDB}/lib )

# handle the QUIETLY and REQUIRED arguments and set *_FOUND
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LevelDb DEFAULT_MSG LEVELDB_LIBRARIES LEVELDB_INCLUDE_DIR)

MARK_AS_ADVANCED(LEVELDB_INCLUDE_DIR LEVELDB_LIBRARIES)

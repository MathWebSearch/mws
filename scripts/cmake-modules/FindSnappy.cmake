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
# SNAPPY_FOUND - system has Snappy
# SNAPPY_INCLUDE_DIR - the Snappy include directory
# SNAPPY_LIBRARIES - Link these to use Snappy
# SNAPPY_DEFINITIONS - Compiler switches required for using Snappy
# SNAPPY_NEED_PREFIX - this is set if the functions are prefixed with Snappy

IF (SNAPPY_INCLUDE_DIR AND SNAPPY_LIBRARIES)
    SET(SNAPPY_FIND_QUIETLY TRUE)
ENDIF (SNAPPY_INCLUDE_DIR AND SNAPPY_LIBRARIES)

FIND_PATH(SNAPPY_INCLUDE_DIR NAMES snappy.h PATH_SUFFIXES snappy HINTS
   /usr/include
   /usr/local/include
   $ENV{SNAPPY}
   $ENV{SNAPPY}/include
   )

FIND_LIBRARY(SNAPPY_LIBRARIES NAMES snappy snappy.dll.a snappy.a HINTS
   /usr/lib
   /usr/local/lib
   $ENV{SNAPPY}
   $ENV{SNAPPY}/lib )

# handle the QUIETLY and REQUIRED arguments and set *_FOUND
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Snappy DEFAULT_MSG SNAPPY_LIBRARIES SNAPPY_INCLUDE_DIR)

MARK_AS_ADVANCED(SNAPPY_INCLUDE_DIR SNAPPY_LIBRARIES)

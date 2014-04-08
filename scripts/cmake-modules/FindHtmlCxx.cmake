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
# Find htmlcxx headers and libraries.
#
#  HTMLCXX_INCLUDE_DIRS - where to find htmlcxx/html/tree.h, etc.
#  HTMLCXX_LIBRARIES    - List of libraries when using HTMLCXX.
#  HTMLCXX_FOUND        - True if HTMLCXX found.

IF (HTMLCXX_INCLUDE_DIRS AND HTMLCXX_LIBRARIES)
    SET(HTMLCXX_FIND_QUIETLY TRUE)
ENDIF (HTMLCXX_INCLUDE_DIRS AND HTMLCXX_LIBRARIES)

FIND_PACKAGE (PkgConfig QUIET)
IF (PKGCONFIG_FOUND)
    PKG_CHECK_MODULES(PC_HTMLCXX QUIET htmlcxx)
ENDIF (PKGCONFIG_FOUND)

FIND_PATH(HTMLCXX_INCLUDE_DIRS
    NAMES
        htmlcxx/html/tree.h
    PATHS
        ${PC_HTMLCXX_INCLUDE_DIRS}
        /usr/local/include
        /usr/include
        $ENV{HTMLCXX}
        $ENV{HTMLCXX}/include
    )

FIND_LIBRARY(HTMLCXX_LIBRARIES
    NAMES
        htmlcxx
    PATHS
        ${PC_HTMLCXX_LIBRARY_DIRS}
        /usr/local/lib
        /usr/lib
        $ENV{HTMLCXX}
        $ENV{HTMLCXX}/lib
    )

# handle the QUIETLY and REQUIRED arguments and set *_FOUND
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(HTMLCXX DEFAULT_MSG HTMLCXX_LIBRARIES HTMLCXX_INCLUDE_DIRS)

MARK_AS_ADVANCED(HTMLCXX_INCLUDE_DIRS HTMLCXX_LIBRARIES)

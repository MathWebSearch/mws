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
# JSON_FOUND - system has LibJson
# JSON_INCLUDE_DIRS - Json include directory
# JSON_LIBRARIES - Link these to use Json

IF (JSON_INCLUDE_DIRS AND JSON_LIBRARIES)
    SET(JSON_FIND_QUIETLY TRUE)
ENDIF (JSON_INCLUDE_DIRS AND JSON_LIBRARIES)

FIND_PACKAGE (PkgConfig QUIET)
IF (PKGCONFIG_FOUND)
    PKG_CHECK_MODULES(PC_JSON QUIET json)
ENDIF (PKGCONFIG_FOUND)

FIND_PATH(JSON_INCLUDE_DIRS
    NAMES
        json-c/json.h
        json-c/json_object.h
    PATHS
        ${PC_JSON_INCLUDE_DIRS}
        /usr/include
        /usr/local/include
        $ENV{JSON}
        $ENV{JSON}/include
    )

FIND_LIBRARY(JSON_LIBRARIES
    NAMES
        json-c json
    PATHS
        ${PC_JSON_LIBRARY_DIRS}
        /usr/lib
        /usr/local/lib
        $ENV{JSON}
        $ENV{JSON}/lib
    )

# handle the QUIETLY and REQUIRED arguments and set *_FOUND
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(JSON DEFAULT_MSG JSON_LIBRARIES JSON_INCLUDE_DIRS)

MARK_AS_ADVANCED(JSON_INCLUDE_DIRS JSON_LIBRARIES)

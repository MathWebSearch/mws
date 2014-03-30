# Copyright (C) 2010-2014 KWARC Group <kwarc.info>
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
# MICROHTTPD_FOUND - system has MicroHttpd
# MICROHTTPD_INCLUDE_DIR - the MicroHttpd include directory
# MICROHTTPD_LIBRARIES - Link these to use MicroHttpd
# MICROHTTPD_DEFINITIONS - Compiler switches required for using MicroHttpd

IF (MICROHTTPD_INCLUDE_DIR AND MICROHTTPD_LIBRARIES)
    SET(MICROHTTPD_FIND_QUIETLY TRUE)
ENDIF (MICROHTTPD_INCLUDE_DIR AND MICROHTTPD_LIBRARIES)

FIND_PACKAGE (PkgConfig)
PKG_CHECK_MODULES (MICROHTTPD QUIET libmicrohttpd)

FIND_PATH(MICROHTTPD_INCLUDE_DIR NAMES microhttpd.h PATH_SUFFIXES microhttpd HINTS
   /usr/include
   /usr/local/include
   $ENV{MICROHTTPD}
   $ENV{MICROHTTPD}/include
   )

FIND_LIBRARY(MICROHTTPD_LIBRARIES NAMES microhttpd microhttpd.dll.a microhttpd.a HINTS
   /usr/lib
   /usr/local/lib
   $ENV{MICROHTTPD}
   $ENV{MICROHTTPD}/lib )

IF (${MICROHTTPD_VERSION} LESS 0.9.3)
    MESSAGE(STATUS "Found MicroHttpd ${MICROHTTPD_VERSION}: deprecated mode")
    ADD_DEFINITIONS("-D_MICROHTTPD_DEPRECATED")
ELSE()
    MESSAGE(STATUS "Found MicroHttpd ${MICROHTTPD_VERSION}")
ENDIF()

# handle the QUIETLY and REQUIRED arguments and set *_FOUND
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MicroHttpd DEFAULT_MSG MICROHTTPD_LIBRARIES MICROHTTPD_INCLUDE_DIR)

MARK_AS_ADVANCED(MICROHTTPD_INCLUDE_DIR MICROHTTPD_LIBRARIES)

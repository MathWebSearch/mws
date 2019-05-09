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
# src/mws/dbc/CMakeLists.txt --
#
# 18 Jun 2011
# c.prodescu@jacobs-university.de
#

# Module name
SET(MODULE "mwsdbc")

# Sources
FILE( GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp" "*.hpp")
ADD_LIBRARY( ${MODULE} ${SOURCES})

# Dependencies
FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(LevelDb REQUIRED)
FIND_PACKAGE(Snappy REQUIRED)

# Includes
TARGET_INCLUDE_DIRECTORIES( ${MODULE} PUBLIC ${LEVELDB_INCLUDE_DIR})
TARGET_INCLUDE_DIRECTORIES( ${MODULE} PRIVATE ${SNAPPY_INCLUDE_DIR})

# Flags

# Binaries
TARGET_LINK_LIBRARIES(${MODULE}
                      mwstypes
                      commontypes
                      commonutils
                      ${LEVELDB_LIBRARIES}
                      ${SNAPPY_LIBRARIES}
                      ${CMAKE_THREAD_LIBS_INIT}
)

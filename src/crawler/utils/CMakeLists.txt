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
# src/mws/crawler/utils/CMakeLists.txt --
#
# 15 Aug 2011
# d.hasegan@jacobs-university.de
#

# Module name
SET(MODULE "crawlerutils")

# Dependencies
FIND_PACKAGE (LibXml2 REQUIRED)
FIND_PACKAGE (PkgConfig REQUIRED)
PKG_SEARCH_MODULE( LIBCURL libcurl REQUIRED )
PKG_SEARCH_MODULE( HTMLCXX htmlcxx REQUIRED )

# Includes
INCLUDE_DIRECTORIES( "${GOOGLEURL_INCLUDES}" )
INCLUDE_DIRECTORIES( "${LIBXML2_INCLUDE_DIR}" )

configure_file ( MwsMathMLTags.txt MwsMathMLTags.txt )
# Flags

# Sources
FILE( GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp" "*.hpp")

# Binaries
ADD_LIBRARY( ${MODULE} ${SOURCES})
TARGET_LINK_LIBRARIES(${MODULE}
                      ${LIBXML2_LIBRARIES}
                      ${LIBCURL_LIBRARIES}
                      ${HTMLCXX_LIBRARIES})


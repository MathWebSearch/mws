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

# Sources
FILE( GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp" "*.c" "*.hpp" "*.h")
ADD_LIBRARY( ${MODULE} ${SOURCES})

# Dependencies
FIND_PACKAGE (LibXml2 REQUIRED)
FIND_PACKAGE (CURL REQUIRED)
FIND_PACKAGE (HtmlCxx REQUIRED)

# Includes
TARGET_INCLUDE_DIRECTORIES( "${MODULE}" PUBLIC "${GOOGLEURL_INCLUDES}" )
TARGET_INCLUDE_DIRECTORIES( "${MODULE}" PRIVATE "${LIBXML2_INCLUDE_DIR}" )
TARGET_INCLUDE_DIRECTORIES( "${MODULE}" PUBLIC "${CURL_INCLUDE_DIRS}" )
TARGET_INCLUDE_DIRECTORIES( "${MODULE}" PUBLIC "${HTMLCXX_INCLUDE_DIRS}" )

configure_file ( MwsMathMLTags.txt MwsMathMLTags.txt )
# Flags


# Binaries
TARGET_LINK_LIBRARIES(${MODULE}
                      ${LIBXML2_LIBRARIES}
                      ${CURL_LIBRARIES}
                      ${HTMLCXX_LIBRARIES}
                      ${GOOGLEURL_LIBRARIES}
                      commonutils
                      )

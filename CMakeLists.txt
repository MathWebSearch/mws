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

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(mathwebsearch CXX C)

SET(${PROJECT_NAME}_MAJOR_VERSION 0)
SET(${PROJECT_NAME}_MINOR_VERSION 8)
SET(${PROJECT_NAME}_PATCH_LEVEL   0)
SET(MWS_VERSION
    ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION} )
SET(MWS_FULL_VERSION
    ${MWS_VERSION}.${${PROJECT_NAME}_PATCH_LEVEL} )


#      Options              Description                     Default
OPTION(APPLY_RESTRICTIONS   "apply restrictions to queries" OFF )
OPTION(WITH_MWS             "build MathWebSearch daemon"    ON )
OPTION(WITH_CRAWLER         "build MWS crawlers"            ON )
OPTION(WITH_DOC             "build MWS documentation"       OFF )

# Select build type
SET(DEFAULT_CMAKE_BUILD_TYPE "Debug")
IF(NOT CMAKE_BUILD_TYPE)
    MESSAGE(STATUS
            "No build type selected, defaults to ${DEFAULT_CMAKE_BUILD_TYPE}")
    SET(CMAKE_BUILD_TYPE "${DEFAULT_CMAKE_BUILD_TYPE}")
ENDIF(NOT CMAKE_BUILD_TYPE)
SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE
    STRING "Choose the type of build: Debug Release RelWithDebInfo MinSizeRel."
    FORCE)

SET(SYSCONFDIR "/etc/mws" CACHE
    STRING "System configuration directory")

SET(LOCALSTATEDIR "/var/lib/mws" CACHE
    STRING "Local state directory")

# Add local CMake module paths
LIST(APPEND CMAKE_MODULE_PATH
    ${PROJECT_SOURCE_DIR}/scripts/cmake-modules
    ${PROJECT_SOURCE_DIR}/third_party/cmake-modules)

# Set compiler flags
SET(COMMON_FLAGS "-Wall -W -Wextra -Wno-flexible-array-extensions")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} -std=c++0x")

# Global include paths
INCLUDE_DIRECTORIES("src"                           # source files
                    "${CMAKE_CURRENT_BINARY_DIR}")  # generated files

ENABLE_TESTING()

ADD_SUBDIRECTORY(config)
ADD_SUBDIRECTORY(scripts)

# Common libraries
ADD_SUBDIRECTORY(third_party/common)
ADD_SUBDIRECTORY(src/common)
ADD_SUBDIRECTORY(test/src/common)

# MWS daemon
IF(WITH_MWS)
    ADD_SUBDIRECTORY(src/mws)
    ADD_SUBDIRECTORY(test/src/mws)
ENDIF(WITH_MWS)

# MWS Crawlers
IF(WITH_CRAWLER)
    ADD_SUBDIRECTORY(third_party/crawler)
    ADD_SUBDIRECTORY(src/crawler)
ENDIF(WITH_CRAWLER)

# Documentation
IF(WITH_DOC)
    FIND_PACKAGE(Doxygen REQUIRED)

    # Add generation method
    ADD_CUSTOM_COMMAND(OUTPUT docs/html/index.html
        COMMAND ${DOXYGEN_EXECUTABLE}
        ARGS ${PROJECT_BINARY_DIR}/config/doxyfile
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM)
    # Set doc as target
    ADD_CUSTOM_TARGET(doc ALL
        DEPENDS ${PROJECT_BINARY_DIR}/docs/html/index.html)
ENDIF(WITH_DOC)

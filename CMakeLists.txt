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
# CMakeLists.txt --
#

# CMake compatibility
CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )

# Project name
PROJECT( mathwebsearch CXX C )

# Project version
SET( ${PROJECT_NAME}_MAJOR_VERSION 0 )
SET( ${PROJECT_NAME}_MINOR_VERSION 6 )
SET( ${PROJECT_NAME}_PATCH_LEVEL   0 )
SET( MWS_VERSION
    ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION} )
SET( MWS_FULL_VERSION
    ${MWS_VERSION}.${${PROJECT_NAME}_PATCH_LEVEL} )


SET(SCRIPTS_DIR "${PROJECT_SOURCE_DIR}/scripts")
SET(PROJECT_CONFIG_DIR
    "${PROJECT_SOURCE_DIR}/config")
SET(MWS_SRC_DIR   # MWS sources
    "${PROJECT_SOURCE_DIR}/src" )
SET(MWS_DATA_DIR
    "${PROJECT_SOURCE_DIR}/data")
SET(MWS_TST_DIR   # MWS Test directory
    "${PROJECT_SOURCE_DIR}/test" )
SET(MWS_TSTSRC_DIR # MWS Test sources directory
    "${MWS_TST_DIR}/src" )
SET(MWS_TSTDAT_DIR # MWS Test data directory
    "${MWS_TST_DIR}/data" )
SET(THIRD_PARTY_DIR
    "${PROJECT_SOURCE_DIR}/third_party")

SET(MATHMLTAGS_PATH ${PROJECT_BINARY_DIR}/src/crawler/utils/MwsMathMLTags.txt)
SET(GETRESPONSE_PATH ${PROJECT_BINARY_DIR}/src/crawler/daemon/GetResponse.html)
# Add local CMake module paths
SET(CMAKE_MODULE_PATH
    "${CMAKE_MODULE_PATH}"
    "${SCRIPTS_DIR}/cmake-modules"
    "${THIRD_PARTY_DIR}/cmake-modules")

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
SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING
    "Choose the type of build: Debug Release RelWithDebInfo MinSizeRel."
    FORCE)

# System configuration directory
SET(SYSCONFDIR "/etc/mws" CACHE
    STRING "System configuration directory")

SET(LOCALSTATEDIR "/var/lib/mws" CACHE
    STRING "Local state directory")

# Compiler flags
SET(COMMON_FLAGS
    "-Wall -W -Wextra -Wno-flexible-array-extensions")
SET(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} ${COMMON_FLAGS}")
SET(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} -std=c++0x")

# Global include paths
INCLUDE_DIRECTORIES( "${MWS_SRC_DIR}"               # source files
                     "${CMAKE_CURRENT_BINARY_DIR}") # generated files

# Generate configuration file
CONFIGURE_FILE(${PROJECT_CONFIG_DIR}/config.h.in
        ${PROJECT_BINARY_DIR}/build-gen/config.h)

CONFIGURE_FILE(${SCRIPTS_DIR}/mws-config.in
    mws-config @ONLY)
INSTALL(PROGRAMS ${PROJECT_BINARY_DIR}/mws-config
    DESTINATION bin)

CONFIGURE_FILE(${SCRIPTS_DIR}/mwsd-service.in
    mwsd-service @ONLY)
INSTALL(PROGRAMS ${PROJECT_BINARY_DIR}/mwsd-service
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${SYSCONFDIR})
INSTALL(CODE
    "FILE(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/${SYSCONFDIR}/configs)")

ENABLE_TESTING()

# Common libraries
ADD_SUBDIRECTORY( "${MWS_SRC_DIR}/common" )
ADD_SUBDIRECTORY( "${MWS_TSTSRC_DIR}/common" )

# MWS daemon
IF ( WITH_MWS )
    ADD_SUBDIRECTORY( "${MWS_SRC_DIR}/mws" )
    ADD_SUBDIRECTORY( "${MWS_TSTSRC_DIR}/mws" )
ENDIF ( WITH_MWS )

# MWS Crawlers
IF ( WITH_CRAWLER )
    ADD_SUBDIRECTORY( "${THIRD_PARTY_DIR}/crawler" )
    ADD_SUBDIRECTORY( "${MWS_SRC_DIR}/crawler" )
ENDIF ( WITH_CRAWLER )

# Documentation
IF ( WITH_DOC )
    FIND_PACKAGE(Doxygen REQUIRED)

    # Generate doxyfile
    INCLUDE(config/Doxyfile)                        # doxyfile

    # Adding doc/ generation method
    ADD_CUSTOM_COMMAND(OUTPUT ${DOXYGEN_OUTPUT_DIR}
            COMMAND ${DOXYGEN_EXECUTABLE}
            ARGS ${CMAKE_CURRENT_BINARY_DIR}/doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen" VERBATIM)
    # Set doc/ as target
    ADD_CUSTOM_TARGET(doc ALL
            DEPENDS ${DOXYGEN_OUTPUT_DIR})
ENDIF ( WITH_DOC )

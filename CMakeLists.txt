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

# Project variables
SET(PROJECT_CONFIG_DIR # Configuration headers and input files
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
SET(MWS_SCRIPT_DIR # MWS Scripts directory
    "${PROJECT_SOURCE_DIR}/scripts" )
SET(THIRD_PARTY_DIR
    "${PROJECT_SOURCE_DIR}/third_party")

SET(MATHMLTAGS_PATH ${PROJECT_BINARY_DIR}/src/crawler/utils/MwsMathMLTags.txt)
SET(GETRESPONSE_PATH ${PROJECT_BINARY_DIR}/src/crawler/daemon/GetResponse.html)
CONFIGURE_FILE(${PROJECT_CONFIG_DIR}/config.h.in
        ${PROJECT_BINARY_DIR}/config.h)

# Add local CMake module paths
SET(CMAKE_MODULE_PATH
    "${CMAKE_MODULE_PATH}"
    "${MWS_SCRIPT_DIR}/cmake-modules"
    "${THIRD_PARTY_DIR}/cmake-modules")

#       Options                 Description                             Default
OPTION( WITH_MWS                "build MathWebSearch daemon"            ON )
OPTION( WITH_REST               "build MWS RESTful interface daemon"    ON )
OPTION( WITH_CRAWLER            "build MWS crawlers"                    OFF )
OPTION( WITH_DOC                "build MWS documentation"               OFF )
OPTION( WITH_TEST               "build test suites"                     ON )
OPTION( CMAKE_BUILD_TYPE "Choose one of Debug|Release|RelWithDebInfo|MinSizeRel"
                                                                    Debug)
# Debug flags
# ADD_DEFINITIONS( "-DTRACE_FUNC_CALLS" )
# Apply restrictions
#ADD_DEFINITIONS( "-D_APPLYRESTRICT" )

# Build Flags
SET(COMMON_FLAGS
    "-Wall -W -Wextra -Wno-flexible-array-extensions")
SET(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} ${COMMON_FLAGS}")
SET(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} -std=c++0x")

# Global include paths
INCLUDE_DIRECTORIES( "${MWS_SRC_DIR}"               # src dir
                     "${PROJECT_CONFIG_DIR}"        # configuration dir
                     "${CMAKE_CURRENT_BINARY_DIR}") # generated files dir

# common libraries
ADD_SUBDIRECTORY( "${MWS_SRC_DIR}/common" )
IF ( WITH_TEST )
    ENABLE_TESTING()
    ADD_SUBDIRECTORY( "${MWS_TSTSRC_DIR}/common" )
ENDIF ( WITH_TEST )

# MWS daemon
IF ( WITH_MWS )
    ADD_SUBDIRECTORY( "${MWS_SRC_DIR}/mws" )
    IF ( WITH_TEST )
        ENABLE_TESTING()
        ADD_SUBDIRECTORY( "${MWS_TSTSRC_DIR}/mws" )
    ENDIF ( WITH_TEST )
ENDIF ( WITH_MWS )

# MWS RESTful interface
IF ( WITH_REST )
    ADD_SUBDIRECTORY( "${MWS_SRC_DIR}/rest" )
ENDIF ( WITH_REST )

# MWS Crawlers
IF ( WITH_CRAWLER )
    ADD_SUBDIRECTORY( "${THIRD_PARTY_DIR}/crawler" )
    ADD_SUBDIRECTORY( "${MWS_SRC_DIR}/crawler" )
ENDIF ( WITH_CRAWLER )

# Configuration scripts -- generating source files
INCLUDE(config/Version)                         # Version.hpp

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

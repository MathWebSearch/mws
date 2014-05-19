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
# Crawler Modules
ADD_SUBDIRECTORY( crawler )             # crawlercrawler
ADD_SUBDIRECTORY( daemon )              # crawlerdaemon
ADD_SUBDIRECTORY( parser )              # crawlerparser
ADD_SUBDIRECTORY( types )               # crawlertypes
ADD_SUBDIRECTORY( utils )               # crawlerutils

# Includes
INCLUDE_DIRECTORIES("${GOOGLEURL_INCLUDES}")
INCLUDE_DIRECTORIES("${JSON_INCLUDE_DIRS}")

# Dependencies
FIND_PACKAGE (LibXml2 REQUIRED)
FIND_PACKAGE (Json REQUIRED)

# Includes
INCLUDE_DIRECTORIES( "${LIBXML2_INCLUDE_DIR}" )
INCLUDE_DIRECTORIES( "${JSON_INCLUDE_DIRS}" )

# MWS stand-alone xhtml harvester
ADD_EXECUTABLE( docs2harvest docs2harvest.cpp)
TARGET_LINK_LIBRARIES( docs2harvest
    crawlerutils
    crawlerparser
    crawlertypes
    commonutils
    ${JSON_LIBRARIES}
)
INSTALL(TARGETS docs2harvest DESTINATION bin)

# docs to elasticsearch json
ADD_EXECUTABLE( harvests2json harvests2json.cpp)
TARGET_LINK_LIBRARIES( harvests2json
        mwsindex
        mwsdaemon
        commonutils
        mwsdbc
        mwsxmlparser
        mwstypes
        ${JSON_LIBRARIES}
        ${LIBXML2_LIBRARIES}
)

INSTALL(TARGETS harvests2json DESTINATION bin)

# MWS crawler daemon
ADD_EXECUTABLE( crawlerd crawlerd.cpp )
TARGET_LINK_LIBRARIES(crawlerd
        googleurl
        crawlerdaemon
        crawlercrawler
        crawlertypes
        crawlerutils
        commonutils
        commonsocket
        commontypes
        commonthread
)

# Output executables at the root of build tree
SET_PROPERTY( TARGET docs2harvest crawlerd harvests2json
        PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

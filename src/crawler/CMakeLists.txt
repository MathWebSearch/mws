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
INCLUDE_DIRECTORIES( "${GOOGLEURL_INCLUDES}" )

# MWS stand-alone xhtml harvester
ADD_EXECUTABLE( docs2harvest docs2harvest.cpp )
TARGET_LINK_LIBRARIES( docs2harvest
    crawlerutils
    crawlerparser
    crawlertypes
    commonutils
)

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
SET_PROPERTY( TARGET docs2harvest crawlerd
        PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

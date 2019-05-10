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

# Sources
FILE( GLOB SOURCES "src/*.cc")

# Targets
ADD_LIBRARY( googleurl ${SOURCES} )

# External dependencies
FIND_PACKAGE( ICU       REQUIRED )
FIND_PACKAGE( Threads   REQUIRED )

# disable warnings from 3rd party code
SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w" )
SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w" )

# Includes
TARGET_INCLUDE_DIRECTORIES( googleurl PUBLIC ${ICU_INCLUDE_DIRS} )

# Dependencies
TARGET_LINK_LIBRARIES( googleurl
                      ${CMAKE_THREAD_LIBS_INIT}
                      ${ICU_LIBRARIES}
)

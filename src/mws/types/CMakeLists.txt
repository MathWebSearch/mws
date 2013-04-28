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
# src/mws/types/CMakeLists.txt --
#
# 18 Jun 2011
# c.prodescu@jacobs-university.de
#

# Module name
SET(MODULE "mwstypes")

# Dependencies

# Includes

# Flags

# Sources
FILE( GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp" "*.hpp")

# Binaries
ADD_LIBRARY( ${MODULE} ${SOURCES})

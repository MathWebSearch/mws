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
# test/src/common/types/CMakeLists.txt --
#
# 10 Dec 2013
# c.prodescu@jacobs-university.de
#

# Dependencies

# Includes

# Flags

# Sources
FILE( GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp" "*.c")

# Binaries
FOREACH(source ${SOURCES})
    GET_FILENAME_COMPONENT(SourceName ${source} NAME_WE)
    # Generate Binaries
    ADD_EXECUTABLE(${SourceName} ${source})
    TARGET_LINK_LIBRARIES(${SourceName}
                          commontypes
                          mwstypes)
    # Add test
    SET(TestName "test_${SourceName}")
    ADD_TEST(${TestName} ${SourceName})
ENDFOREACH(source)

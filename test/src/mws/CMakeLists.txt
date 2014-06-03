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
ADD_SUBDIRECTORY( dbc )
ADD_SUBDIRECTORY( index )
ADD_SUBDIRECTORY( parser )
ADD_SUBDIRECTORY( query )

CONFIGURE_FILE(./mws-integration-test.sh.in
    mws-integration-test.sh @ONLY)
ADD_TEST(NAME integration_qvar_query
    COMMAND mws-integration-test.sh)
SET_TESTS_PROPERTIES(integration_qvar_query PROPERTIES TIMEOUT 10)

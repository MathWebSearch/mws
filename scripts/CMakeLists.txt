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
CONFIGURE_FILE(mws-config.in mws-config @ONLY)
INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/mws-config
    DESTINATION bin)

CONFIGURE_FILE(mwsd-service.sh.in mwsd-service.sh @ONLY)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/mwsd-service.sh
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${SYSCONFDIR})

INSTALL(DIRECTORY
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${SYSCONFDIR}/configs)

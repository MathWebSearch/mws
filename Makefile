#
# Copyright (C) 2010-2014 KWARC Group <kwarc.info>
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
# Makefile --
#

all: bin/cmake_bootstrap_success
	@cd bin && make --no-print-directory $@

clean:
	@rm -rf bin/ build/

config: bin/CMakeCache.txt
	@ccmake bin/

test: bin/cmake_bootstrap_success all
	@cd bin && make --no-print-directory $@

# CMake setup
bin/CMakeCache.txt:
	@mkdir -p bin
	-@cd bin && cmake ..

# CMake successful setup
bin/cmake_bootstrap_success:
	@mkdir -p bin
	@cd bin && cmake ..
	@touch bin/cmake_bootstrap_success

# Forward targets to cmake generated makefile
%: bin/cmake_bootstrap_success
	@cd bin && make --no-print-directory $@

.PHONY: all clean config test
.SECONDARY: bin/cmake_bootstrap_success

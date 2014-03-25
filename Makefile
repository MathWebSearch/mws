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

all: build/cmake_done_config
	@cd build && make --no-print-directory $@

clean:
	@rm -rf build

%: build/cmake_done_config
	@cd build && make --no-print-directory $@

config: build/cmake_done_config
	@ccmake build/

test: build/cmake_done_config all
	@cd build && make --no-print-directory $@

build/cmake_done_config:
	@mkdir -p build
	@cd build && cmake ..
	@touch build/cmake_done_config

.PHONY: all clean config test

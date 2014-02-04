/*

Copyright (C) 2010-2013 KWARC Group <kwarc.info>

This file is part of MathWebSearch.

MathWebSearch is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MathWebSearch is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MathWebSearch.  If not, see <http://www.gnu.org/licenses/>.

*/

/**
  * @file util.cpp
  * @brief Utilities implementation
  * @date 22 Nov 2013
  *
  * License: GPL v3
  */


#include <errno.h>
#include <string.h>

#include <fstream>
#include <stdexcept>

#include "util.hpp"

using namespace std;

namespace common { namespace utils {

std::string
getFileContents(const std::string& path) {
    string contents;

    try {
        ifstream in(path, std::ios::in | std::ios::binary);
        if (in.good()) {
            in.seekg(0, std::ios::end);
            contents.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(&contents[0], contents.size());
            in.close();
        }
    } catch (...) {
        throw std::runtime_error(strerror(errno));
    }

    return contents;
}

} }


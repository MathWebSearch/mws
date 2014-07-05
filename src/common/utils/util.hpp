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
#ifndef _COMMON_UTILS_UTIL_HPP
#define _COMMON_UTILS_UTIL_HPP

/**
  * @file util.hpp
  * @brief Utilities API
  * @date 22 Nov 2013
  *
  * License: GPL v3
  */

#include <stdexcept>
#include <string>
#include <functional>

struct Tester;
#define ALLOW_TESTER_ACCESS friend struct ::Tester

namespace common {
namespace utils {

// String utils

bool hasSuffix(const std::string& str, const std::string& suffix);

/**
 * @brief Remove duplicate spaces from a string and replace all spaces with ' '
 * @param str string to remove spaces from
 */
void removeDuplicateSpaces(std::string* str);

// Directory Structure

std::string getFileContents(const std::string& path) throw(std::runtime_error);

typedef std::function<
    int(const std::string& fullPath, const std::string& directoryPartialPath)>
    FileCallback;
typedef std::function<bool(const std::string& directoryPartialPath)>
    DirectoryCallback;

static DirectoryCallback ignore_directories = [](
    const std::string partialPath) {
    printf("Skipping directory %s\n", partialPath.c_str());
    return false;
};

int foreachEntryInDirectory(const std::string& path,
                            const FileCallback& fileCallback,
                            const DirectoryCallback& directoryCallback =
                                ignore_directories);

std::string formattedString(const char* fmt, ...)
    __attribute__((format(printf, 1, 2)));
std::string humanReadableByteCount(uint64_t bytes, bool si);

/**
 * @brief create directory at the specified path
 * @param path where to create directory
 */
void create_directory(const std::string& path,
                      bool error_if_exists = false) /* throw runtime_error */;

}  // namespace utils
}  // namespace common

#endif  // _COMMON_UTILS_UTIL_HPP

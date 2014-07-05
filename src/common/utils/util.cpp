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
  * @edit Radu Hambasan
  * @date 09 Jun 2014
  *
  * License: GPL v3
  */

#include <assert.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
using std::sort;
#include <cinttypes>
#include <functional>
using std::function;
#include <queue>
using std::queue;
#include <stdexcept>
using std::runtime_error;
#include <sstream>
using std::stringstream;
#include <string>
using std::string;
#include <vector>
using std::vector;

#include "common/utils/memstream.h"
#include "common/utils/compiler_defs.h"

#include "util.hpp"

namespace common {
namespace utils {

bool hasSuffix(const std::string& str, const std::string& suffix) {
    return (str.length() >= suffix.length()) &&
           (0 == str.compare(str.length() - suffix.length(), suffix.length(),
                             suffix));
}

void removeDuplicateSpaces(string* str) {
    stringstream input(*str);
    stringstream output;
    string token;

    bool shouldSeparate = false;
    while (input >> token) {
        if (shouldSeparate) output << " ";
        output << token;
        shouldSeparate = true;
    }

    *str = output.str();
}

std::string getFileContents(const std::string& path) throw(runtime_error) {
    int fd = -1;
    off_t curr, size;
    struct stat file_stat;
    string contents;

    FAIL_ON((fd = open(path.c_str(), O_RDONLY)) < 0);
    FAIL_ON(fstat(fd, &file_stat) < 0);

    curr = 0;
    size = file_stat.st_size;
    contents.resize(size);
    do {
        ssize_t ret;
        FAIL_ON((ret = read(fd, &contents[curr], size - curr)) < 0);
        curr += ret;
    } while (curr < size);
    FAIL_ON(curr != size);

    close(fd);

    return contents;

fail:
    if (fd >= 0) close(fd);
    assert(errno != 0);
    throw runtime_error(path + ": " + strerror(errno));
}

struct FileData {
    string relativePath;
    string directoryPartialPath;

    FileData(string relativePath, string directoryPartialPath)
        : relativePath(std::move(relativePath)),
          directoryPartialPath(std::move(directoryPartialPath)) {}
};

static int getPathsInDirectory(const std::string& directoryPath,
                               const std::string& prefix,
                               queue<string>* directoriesOut,
                               queue<FileData>* filesOut) {
    DIR* directory;
    struct dirent entryData, *currEntry;
    vector<string> directories;
    vector<string> fileRelativePaths;

    FAIL_ON((directory = opendir(directoryPath.c_str())) == nullptr);
    while (1) {
        FAIL_ON(readdir_r(directory, &entryData, &currEntry) != 0);
        if (currEntry == nullptr) {
            // end of directory
            break;
        }
        const string fullPath = directoryPath + "/" + currEntry->d_name;
        const string relativePath = prefix + "/" + currEntry->d_name;
        if (currEntry->d_name[0] == '.') {
            PRINT_LOG("Skipping \"%s\": hidden\n", fullPath.c_str());
        } else {
            switch (currEntry->d_type) {
            case DT_DIR:
                directories.push_back(relativePath);
                break;
            case DT_REG:
                fileRelativePaths.push_back(relativePath);
                break;
            default:
                PRINT_LOG("Skipping \"%s\": not a regular file\n",
                          fullPath.c_str());
                break;
            }
        }
    }
    FAIL_ON(closedir(directory) != 0);

    sort(directories.begin(), directories.end(), std::less<string>());
    sort(fileRelativePaths.begin(), fileRelativePaths.end(),
         std::less<string>());

    for (string fileRelativePath : fileRelativePaths) {
        filesOut->push(FileData(fileRelativePath, prefix));
    }
    for (string directory : directories) {
        directoriesOut->push(directory);
    }
    return 0;

fail:
    PRINT_WARN("%s\n", directoryPath.c_str());
    return -1;
}

int foreachEntryInDirectory(const std::string& path,
                            const FileCallback& fileCallback,
                            const DirectoryCallback& directoryCallback) {
    queue<string> directories;
    queue<FileData> files;

    FAIL_ON(getPathsInDirectory(path, ".", &directories, &files) != 0);

    while (!directories.empty() || !files.empty()) {
        while (!files.empty()) {
            const FileData fileData = files.front();
            files.pop();
            FAIL_ON(fileCallback(path + "/" + fileData.relativePath,
                                 fileData.directoryPartialPath) != 0);
        }
        if (!directories.empty()) {
            const string directory = directories.front();
            directories.pop();
            if (directoryCallback(directory)) {
                FAIL_ON(getPathsInDirectory(path + "/" + directory, directory,
                                            &directories, &files) != 0);
            }
        }
    }

    return 0;

fail:
    return -1;
}

string formattedString(const char* fmt, ...) {
    char* buffer;
    size_t size;
    FILE* stream = open_memstream(&buffer, &size);
    va_list va;
    va_start(va, fmt);
    vfprintf(stream, fmt, va);
    va_end(va);
    fclose(stream);
    string result(buffer, size);
    free(buffer);

    return result;
}

string humanReadableByteCount(uint64_t bytes, bool si) {
    unsigned unit = si ? 1000 : 1024;
    if (bytes < unit) {
        return formattedString("%" PRIu64 " B", bytes);
    }
    int exp = static_cast<int>(log(bytes) / log(unit));

    assert(exp <= 6);
    if (exp > 6) {
        exp = 6;
    }
    string prefix = (si ? "kMGTPE" : "KMGTPE")[exp - 1] + string(si ? "" : "i");

    return formattedString("%.1f %sB", bytes / pow(unit, exp), prefix.c_str());
}

void create_directory(const string& path, bool error_if_exists) {
    if (access(path.c_str(), F_OK) == 0) {  // path exists
        if (error_if_exists) {
            throw runtime_error(
                formattedString("path \"%s\" already exists", path.c_str()));
        }

        struct stat status;
        stat(path.c_str(), &status);

        if (status.st_mode & S_IFDIR) {
            // Test read and write access
            if (access(path.c_str(), R_OK | W_OK) != 0) {
                throw runtime_error(
                    formattedString("path \"%s\" is not RW accessible", path.c_str()));
            }
        } else {
            throw runtime_error(
                formattedString("path \"%s\" is not a directory", path.c_str()));
        }
    } else {
        if (mkdir(path.c_str(), 0755) < 0) {
            throw runtime_error(
                formattedString("%s: %s", path.c_str(), strerror(errno)));
        }
    }
}

}  // namespace utils
}  // namespace common

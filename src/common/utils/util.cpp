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

#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <queue>
using std::queue;
#include <string>
using std::string;
#include <fstream>
using std::ifstream;
#include <stdexcept>
using std::runtime_error;
#include <functional>

#include "common/utils/macro_func.h"
#include "util.hpp"


namespace common { namespace utils {

bool hasSuffix(const std::string& str, const std::string& suffix) {
    return (str.length() >= suffix.length()) &&
            (0 == str.compare(str.length() - suffix.length(),
                              suffix.length(), suffix));
}

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
        throw runtime_error(strerror(errno));
    }

    return contents;
}

static int getPathsInDirectory(const std::string& directoryPath,
                               const std::string& prefix,
                               queue<string>* directories,
                               queue<string>* files) {
    DIR* directory;
    struct dirent entryData, *currEntry;

    FAIL_ON((directory = opendir(directoryPath.c_str())) == NULL);
    while (1) {
        FAIL_ON(readdir_r(directory, &entryData, &currEntry) != 0);
        if (currEntry == NULL) {
            // end of directory
            break;
        }
        const string name = prefix + "/" + currEntry->d_name;
        if (currEntry->d_name[0] == '.') {
            printf("Skipping entry \"%s\": hidden\n", name.c_str());
        } else {
            switch (currEntry->d_type) {
            case DT_DIR:
                directories->push(name);
                break;
            case DT_REG:
                files->push(name);
                break;
            default:
                printf("Skiping entry \"%s\": not a regular file\n",
                       name.c_str());
                break;
            }
        }
    }
    FAIL_ON(closedir(directory) != 0);

    return 0;

fail:
    return -1;
}

int
foreachEntryInDirectory(const std::string& path,
                        const FileCallback& fileCallback,
                        const DirectoryCallback& directoryCallback) {
    queue<string> directories, files;

    FAIL_ON(getPathsInDirectory(path, ".", &directories, &files) != 0);

    while (!directories.empty() || !files.empty()) {
        while (!files.empty()) {
            const string file = files.front();
            files.pop();
            FAIL_ON(fileCallback(path + "/" + file) != 0);
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

}  // namespace utils
}  // namespace common


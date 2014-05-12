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

#include <algorithm>
#include <fstream>
using std::ifstream;
#include <functional>
using std::function;
#include <queue>
using std::queue;
#include <stdexcept>
using std::runtime_error;
#include <string>
using std::string;
#include <vector>
using std::vector;

#include "common/utils/compiler_defs.h"

#include "util.hpp"

namespace common { namespace utils {

bool hasSuffix(const std::string& str, const std::string& suffix) {
    return (str.length() >= suffix.length()) &&
            (0 == str.compare(str.length() - suffix.length(),
                              suffix.length(), suffix));
}

void removeDuplicateSpaces(string* str) {
    function<bool(char, char)> BothAreSpaces = [](char lhs, char rhs) {
        return isspace(lhs) && isspace(rhs);
    };
    function<bool(char)> IsSpace = [](char ch) {
        return isspace(ch);
    };
    replace_if(str->begin(), str->end(), IsSpace, ' ');
    string::iterator new_end = unique(str->begin(), str->end(), BothAreSpaces);
    str->erase(new_end, str->end());
}

std::string
getFileContents(const std::string& path) throw (runtime_error) {
    try {
        string contents;
        ifstream file;
        file.exceptions(ifstream::failbit | ifstream::badbit);
        // Find file size
        file.open(path, std::ios::in | std::ios::binary);
        file.seekg(0, std::ios::end);
        contents.resize(file.tellg());
        // Rewind file
        file.seekg(0, std::ios::beg);
        // Read into string
        file.read(&contents[0], contents.size());
        file.close();
        return contents;
    } catch (...) {
        throw runtime_error(path + ": " + strerror(errno));
    }
}

struct FileData {
    string fullPath;
    string directoryPartialPath;

    FileData(const string& fullPath, const string& directoryPartialPath)
        : fullPath(fullPath), directoryPartialPath(directoryPartialPath) {
    }
};

static int getPathsInDirectory(const std::string& directoryPath,
                               const std::string& prefix,
                               queue<string>* directoriesOut,
                               queue<FileData>* filesOut) {
    DIR* directory;
    struct dirent entryData, *currEntry;
    vector<string> directories;
    vector<string> files;

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
                directories.push_back(name);
                break;
            case DT_REG:
                files.push_back(name);
                break;
            default:
                printf("Skiping entry \"%s\": not a regular file\n",
                       name.c_str());
                break;
            }
        }
    }
    FAIL_ON(closedir(directory) != 0);

    sort(directories.begin(), directories.end(), std::less<string>());
    sort(files.begin(), files.end(), std::less<string>());

    for (string file : files) {
        filesOut->push(FileData(file, prefix));
    }
    for (string directory : directories) {
        directoriesOut->push(directory);
    }
    return 0;

fail:
    PRINT_WARN("%s\n", directoryPath.c_str());
    return -1;
}

int
foreachEntryInDirectory(const std::string& path,
                        const FileCallback& fileCallback,
                        const DirectoryCallback& directoryCallback) {
    queue<string> directories;
    queue<FileData> files;

    FAIL_ON(getPathsInDirectory(path, ".", &directories, &files) != 0);

    while (!directories.empty() || !files.empty()) {
        while (!files.empty()) {
            const FileData fileData = files.front();
            files.pop();
            FAIL_ON(fileCallback(path + "/" + fileData.fullPath,
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

}  // namespace utils
}  // namespace common


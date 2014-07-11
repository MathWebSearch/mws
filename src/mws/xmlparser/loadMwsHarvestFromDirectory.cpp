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
  * @file loadMwsHarvestFromDirectory.cpp
  * @date 30 Apr 2012
  *
  * License: GPL v3
  *
  */

// System includes

#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <string>
#include <vector>
#include <utility>

#include "common/utils/compiler_defs.h"
#include "common/utils/Path.hpp"
#include "common/utils/util.hpp"
using common::utils::foreachEntryInDirectory;
#include "processMwsHarvest.hpp"

// Namespaces

using namespace std;

namespace mws {
namespace parser {

uint64_t loadHarvests(mws::index::IndexBuilder* indexBuilder,
                 const index::HarvesterConfiguration& config) {
    uint64_t numExpressions = 0;

    for (string dirPath : config.paths) {
        PRINT_LOG("Loading from %s...\n", dirPath.c_str());
        common::utils::FileCallback
                fileCallback = [&](
                const std::string& path, const std::string& prefix) {
            UNUSED(prefix);
            if (common::utils::hasSuffix(path, config.fileExtension)) {
                PRINT_LOG("Loading %s... ", path.c_str());
                int fd = open(path.c_str(), O_RDONLY);
                if (fd < 0) {
                    perror(path.c_str());
                    return -1;
                }
                auto loadReturn = loadMwsHarvestFromFd(indexBuilder, fd);
                if (loadReturn.first == 0) {
                    PRINT_LOG("%d loaded\n", loadReturn.second);
                } else {
                    PRINT_LOG("%d loaded (with errors)\n", loadReturn.second);
                }
                numExpressions += loadReturn.second;
                close(fd);
            } else {
                PRINT_LOG("Skipping \"%s\": bad extension\n", path.c_str());
            }

            return 0;
        };

        common::utils::DirectoryCallback recursive = [&](
                const std::string partialPath) {
            UNUSED(partialPath);
            return config.recursive;
        };

        if (foreachEntryInDirectory(dirPath, fileCallback, recursive) != 0) {
            continue;
        }
    }

    return numExpressions;
}

}  // namespace parser
}  // namespace mws

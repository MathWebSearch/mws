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
  * @brief File containing the implementation of the loadMwsHarvestFromDirectory
  * function
  *
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
#include <utility>

#include "common/utils/compiler_defs.h"
#include "common/utils/Path.hpp"
#include "common/utils/util.hpp"
#include "mws/index/IndexManager.hpp"
using mws::index::IndexingOptions;
#include "processMwsHarvest.hpp"


// Namespaces

using namespace std;

namespace mws {
namespace parser {

int
loadMwsHarvestFromDirectory(mws::index::IndexManager* indexManager,
                            mws::AbsPath const& dirPath,
                            const std::string& extension,
                            bool recursive) {
    int totalLoaded = 0;

    common::utils::FileCallback fileCallback =
            [&totalLoaded, indexManager, extension]
            (const std::string& path, const std::string& prefix) {
        UNUSED(prefix);
        if (common::utils::hasSuffix(path, extension)) {
            printf("Loading %s... ", path.c_str());
            int fd = open(path.c_str(), O_RDONLY);
            if (fd < 0) {
                return -1;
            }
            auto loadReturn = loadMwsHarvestFromFd(indexManager, fd);
            if (loadReturn.first == 0) {
                printf("%d loaded\n", loadReturn.second);
            } else {
                printf("%d loaded (with errors)\n", loadReturn.second);
            }
            totalLoaded += loadReturn.second;
            close(fd);
        } else {
            printf("Skipping \"%s\": bad extension\n", path.c_str());
        }

        return 0;
    };
    common::utils::DirectoryCallback shouldRecurse =
            [](const std::string partialPath) {
        UNUSED(partialPath);
        return true;
    };

    printf("Loading harvest files...\n");
    if (recursive) {
        FAIL_ON(common::utils::foreachEntryInDirectory(dirPath.get(),
                                                       fileCallback,
                                                       shouldRecurse));
    } else {
        FAIL_ON(common::utils::foreachEntryInDirectory(dirPath.get(),
                                                       fileCallback));
    }

fail:
    return totalLoaded;
}

}  // namespace parser
}  // namespace mws

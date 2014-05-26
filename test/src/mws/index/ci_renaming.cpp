/*

Copyright (C) 2010-2014 KWARC Group <kwarc.info>

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
  * @brief Test if <m:ci> token renaming maintains data integrity
  *
  * @file ci_renaming.cpp
  * @author Radu Hambasan
  * @date 07 Apr 2014
  *
  * License: GPL v3
  *
  */

// System includes

#include <sys/types.h>      // Primitive System datatypes
#include <sys/stat.h>       // POSIX File characteristics
#include <fcntl.h>          // File control operations
#include <libxml/parser.h>  // LibXML parser header
#include <stdlib.h>

#include <string>  // C++ String header
#include <utility>

// Local includes

#include "mws/dbc/MemCrawlDb.hpp"
#include "mws/dbc/MemFormulaDb.hpp"
#include "mws/index/MeaningDictionary.hpp"
using mws::index::MeaningDictionary;
#include "mws/xmlparser/initxmlparser.hpp"
#include "mws/xmlparser/clearxmlparser.hpp"
#include "mws/xmlparser/processMwsHarvest.hpp"
#include "mws/dbc/MemCrawlDb.hpp"
#include "mws/dbc/MemFormulaDb.hpp"
#include "common/utils/compiler_defs.h"

#include "build-gen/config.h"

// Namespaces

/*
 * In the test harvest there are 2 <expr>'s:
 * x + y
 * and
 * a + b
 * we expect to get 8 expressions
 * and the index size to be only 4
 */

using namespace std;
using namespace mws;

struct Tester {
    static inline bool ci_renaming_successful() {
        dbc::MemCrawlDb crawlDb;
        dbc::MemFormulaDb formulaDb;
        index::TmpIndex data;
        MeaningDictionary meaningDictionary;
        index::IndexingOptions indexingOptions;
        indexingOptions.renameCi = true;
        index::IndexBuilder indexBuilder(&formulaDb, &crawlDb, &data,
                                         &meaningDictionary, indexingOptions);
        const string harvest_path =
            (string)MWS_TESTDATA_PATH + "/ci_renaming.harvest";
        int fd;
        std::pair<int, int> ret;
        FAIL_ON(initxmlparser() != 0);
        FAIL_ON((fd = open(harvest_path.c_str(), O_RDONLY)) < 0);
        ret = parser::loadMwsHarvestFromFd(&indexBuilder, fd);
        // Fail if the parsing was not sucessful
        FAIL_ON(ret.first != 0);
        // Fail if we have not indexed all expresions
        FAIL_ON(ret.second != 8);
        // Fail if the index is not as compressed as it should
        FAIL_ON(data.mRoot->children.size() != 4);
        (void)close(fd);

        (void)clearxmlparser();

        return true;

    fail:
        return false;
    }
};

int main() {
    if (Tester::ci_renaming_successful()) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}

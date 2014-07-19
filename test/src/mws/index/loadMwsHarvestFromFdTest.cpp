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
  * @brief loadMwsHarvestFromFd Test
  *
  * @file loadMwsHarvestFromFdTest.cpp
  * @author Prodescu Corneliu-Claudiu <c.prodescu@jacobs-university.de>
  * @date 30 Apr 2011
  *
  * License: GPL v3
  *
  */

#include <sys/types.h>      // Primitive System datatypes
#include <sys/stat.h>       // POSIX File characteristics
#include <fcntl.h>          // File control operations
#include <libxml/parser.h>  // LibXML parser header
#include <stdlib.h>

#include <cinttypes>
#include <string>
using std::string;
#include <vector>
using std::vector;

#include "mws/index/MeaningDictionary.hpp"
using mws::index::MeaningDictionary;
#include "mws/index/TmpIndex.hpp"
using mws::index::TmpIndex;
#include "mws/index/IndexBuilder.hpp"
using mws::index::IndexBuilder;
#include "mws/index/ExpressionEncoder.hpp"
using mws::index::ExpressionEncoder;
#include "mws/xmlparser/xmlparser.hpp"
using mws::parser::initxmlparser;
#include "mws/dbc/MemCrawlDb.hpp"
using mws::dbc::MemCrawlDb;
#include "mws/dbc/MemFormulaDb.hpp"
using mws::dbc::MemFormulaDb;
#include "common/utils/compiler_defs.h"

#include "build-gen/config.h"

struct HarvestData {
    string path;
    int returnValue;
    uint64_t expressionCount;
};

const vector<HarvestData> harvests = {{"empty.harvest", -1, 0},
                                      {"eq_ambiguity.harvest", 0, 4},
                                      {"data1.harvest", 0, 17},
                                      {"data2.harvest", 0, 14},
                                      {"data3.harvest", 0, 14},
                                      {"data4.harvest", 0, 1}};

int main() {
    MemCrawlDb crawlDb;
    MemFormulaDb formulaDb;
    TmpIndex data;
    MeaningDictionary meaningDictionary;
    ExpressionEncoder::Config indexingOptions;
    indexingOptions.renameCi = false;
    IndexBuilder indexBuilder(&formulaDb, &crawlDb, &data, &meaningDictionary,
                              indexingOptions);
    FAIL_ON(initxmlparser() != 0);

    for (HarvestData harvest : harvests) {
        const string harvest_path =
            (string)MWS_TESTDATA_PATH + "/" + harvest.path;
        int fd;

        FAIL_ON((fd = open(harvest_path.c_str(), O_RDONLY)) < 0);
        auto ret = loadHarvestFromFd(&indexBuilder, fd);
        FAIL_ON(ret.status != harvest.returnValue);
        FAIL_ON(ret.numExpressions != harvest.expressionCount);

        (void)close(fd);
    }

    return EXIT_SUCCESS;

fail:
    return EXIT_FAILURE;
}

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
  * @file mws-index.cpp
  * @brief mws-index executable
  * @author Radu Hambasan
  * @date 18 Jan 2014
  *
  * @edit Radu Hambasan
  * @date 29 May 2014
  */

#include <cinttypes>
#include <stdexcept>
using std::exception;
#include <string>
using std::string;
#include <fstream>
#include <memory>
using std::unique_ptr;

#include "common/utils/util.hpp"
using common::utils::humanReadableByteCount;
using common::utils::create_directory;
#include "mws/dbc/LevCrawlDb.hpp"
#include "mws/dbc/LevFormulaDb.hpp"
#include "mws/index/TmpIndex.hpp"
#include "mws/index/memsector.h"
#include "mws/index/IndexBuilder.hpp"
#include "mws/index/MeaningDictionary.hpp"
#include "mws/index/IndexWriter.hpp"

#include "build-gen/config.h"

namespace mws {
namespace index {

int createCompressedIndex(const IndexConfiguration& config) {
    const std::string output_dir = config.dataPath;
    memsector_writer_t mwsr;
    unique_ptr<dbc::CrawlDb> crawlDb;
    unique_ptr<dbc::FormulaDb> formulaDb;
    TmpIndex index;
    unique_ptr<MeaningDictionary> meaningDictionary;
    unique_ptr<index::IndexBuilder> indexBuilder;
    std::filebuf fb;
    std::ostream os(&fb);
    uint64_t numExpressions;

    try {
        create_directory(output_dir);
        auto crawlLevDb = new dbc::LevCrawlDb();
        crawlLevDb->create_new((output_dir + "/" + CRAWL_DB_FILE).c_str(),
                               config.deleteOldData);
        crawlDb.reset(crawlLevDb);
        auto formulaLevDb = new dbc::LevFormulaDb();
        formulaLevDb->create_new((output_dir + "/" + FORMULA_DB_FILE).c_str(),
                                 config.deleteOldData);
        formulaDb.reset(formulaLevDb);
    } catch (exception& e) {
        PRINT_WARN("%s\n", e.what());
        goto failure;
    }
    meaningDictionary.reset(new MeaningDictionary());

    indexBuilder.reset(new index::IndexBuilder(formulaDb.get(), crawlDb.get(),
                                               &index, meaningDictionary.get(),
                                               config.encoding));
    numExpressions = loadHarvests(indexBuilder.get(), config.harvester);
    if (numExpressions == 0) {
        PRINT_WARN("No expressions loaded. Aborting...\n");
        goto failure;
    }
    PRINT_LOG("%" PRIu64 " expressions loaded.\n", numExpressions);

    memsector_create(&mwsr, (output_dir + "/" + INDEX_MEMSECTOR_FILE).c_str());
    index.exportToMemsector(&mwsr);
    PRINT_LOG("Created index of %s\n",
              humanReadableByteCount(mwsr.ms.index_size,
                                     /* si= */ false).c_str());

    fb.open((output_dir + "/" + MEANING_DICTIONARY_FILE).c_str(),
            std::ios::out);
    meaningDictionary->save(os);
    fb.close();

    return EXIT_SUCCESS;

failure:
    return EXIT_FAILURE;
}

}  // namespace index
}  // namespace mws

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

#include "mws/dbc/LevCrawlDb.hpp"
#include "mws/dbc/LevFormulaDb.hpp"
#include "mws/index/TmpIndex.hpp"
using mws::index::TmpIndex;
#include "mws/index/memsector.h"
#include "mws/index/MeaningDictionary.hpp"
using mws::index::MeaningDictionary;
#include "mws/xmlparser/processMwsHarvest.hpp"
using mws::parser::loadMwsHarvestFromDirectory;
#include "mws/xmlparser/clearxmlparser.hpp"
#include "IndexBuilder.hpp"
using mws::clearxmlparser;

#include "mws/index/IndexWriter.hpp"

namespace mws {
namespace index {

int createCompressedIndex(const IndexingConfiguration& config) {
    const std::string output_dir = config.dataPath;
    const IndexingOptions indexingOptions = config.indexingOptions;
    memsector_writer_t mwsr;
    dbc::CrawlDb* crawlDb;
    dbc::FormulaDb* formulaDb;
    TmpIndex index;
    MeaningDictionary* meaningDictionary;
    index::IndexBuilder* indexBuilder;
    std::filebuf fb;
    std::ostream os(&fb);

    // if the path exists
    if (access(output_dir.c_str(), F_OK) == 0) {
        struct stat status;
        stat(output_dir.c_str(), &status);

        if (status.st_mode & S_IFDIR) {
            // Everything ok if the folder has no other files called
            // crawl.db, level.db, memsector.dat or meaning.dat
        } else {
            fprintf(stderr, "The path you entered is a file");
            goto failure;
        }
    } else {
        mkdir(output_dir.c_str(), 0755);
    }

    try {
        auto crawlLevDb = new dbc::LevCrawlDb();
        crawlLevDb->create_new((output_dir + "/" + CRAWL_DB_FILE).c_str(),
                               /* deleteIfExists = */ false);
        crawlDb = crawlLevDb;
        auto formulaLevDb = new dbc::LevFormulaDb();
        formulaLevDb->create_new((output_dir + "/" + FORMULA_DB_FILE).c_str(),
                                 /* deleteIfExists = */ false);
        formulaDb = formulaLevDb;
    } catch (exception& e) {
        PRINT_WARN("%s\n", e.what());
        goto failure;
    }
    meaningDictionary = new MeaningDictionary();

    indexBuilder = new index::IndexBuilder(formulaDb, crawlDb, &index,
                                           meaningDictionary, indexingOptions);
    for (string it : config.harvestLoadPaths) {
        AbsPath harvestPath(it);
        printf("Loading from %s...\n", it.c_str());
        printf("%d expressions loaded.\n",
               parser::loadMwsHarvestFromDirectory(indexBuilder, harvestPath,
                                                   config.harvestFileExtension,
                                                   config.recursive));
        fflush(stdout);
    }
    memsector_create(&mwsr, (output_dir + "/" + INDEX_MEMSECTOR_FILE).c_str());
    index.exportToMemsector(&mwsr);
    PRINT_LOG("Created index of %" PRIu64 "Kb\n", mwsr.ms.index_size / 1024);

    fb.open((output_dir + "/" + MEANING_DICTIONARY_FILE).c_str(),
            std::ios::out);
    meaningDictionary->save(os);
    fb.close();

    clearxmlparser();
    delete crawlDb;
    delete formulaDb;
    delete meaningDictionary;
    delete indexBuilder;

    PRINT_LOG("Index saved to %s\n", output_dir.c_str());

    return EXIT_SUCCESS;

failure:
    return EXIT_FAILURE;
}

}  // namespace index
}  // namespace mws

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
#include "common/utils/util.hpp"
using common::utils::getDirectorySize;
using common::utils::humanReadableByteCount;

#include "mws/index/IndexWriter.hpp"

namespace mws {
namespace index {

int createCompressedIndex(const IndexingConfiguration& config) {
    const std::string output_dir = config.dataPath;
    const IndexingOptions indexingOptions = config.indexingOptions;
    memsector_writer_t mwsr;
    unique_ptr<dbc::CrawlDb> crawlDb;
    unique_ptr<dbc::FormulaDb> formulaDb;
    TmpIndex index;
    unique_ptr<MeaningDictionary> meaningDictionary;
    unique_ptr<index::IndexBuilder> indexBuilder;
    std::filebuf fb;
    std::ostream os(&fb);

    size_t invalid_paths = 0;

    // if the path exists
    if (access(output_dir.c_str(), F_OK) == 0) {
        struct stat status;
        stat(output_dir.c_str(), &status);

        if (status.st_mode & S_IFDIR) {
            // Do we have write access?
            if (access(output_dir.c_str(), W_OK) == 0) {
                // Everything ok if the folder has no other files called
                // crawl.db, level.db, index.memsector or meanings.dat
            } else {
                PRINT_WARN("%s: Permission denied\n", output_dir.c_str());
                goto failure;
            }
        } else {
            PRINT_WARN("The path you entered is a file\n");
            goto failure;
        }
    } else {
        // create the directory (because it doesn't exist)
        mkdir(output_dir.c_str(), 0755);
        // Do we have write access, now that we have created it?
        if (access(output_dir.c_str(), W_OK) == 0) {
            // Everything ok.
        } else {
            // We were not able to create the file
            PRINT_WARN("%s: Permission denied.\n", output_dir.c_str());
            goto failure;
        }
    }

    try {
        auto crawlLevDb = new dbc::LevCrawlDb();
        crawlLevDb->create_new((output_dir + "/" + CRAWL_DB_FILE).c_str(),
                               /* deleteIfExists = */ false);
        crawlDb.reset(crawlLevDb);
        auto formulaLevDb = new dbc::LevFormulaDb();
        formulaLevDb->create_new((output_dir + "/" + FORMULA_DB_FILE).c_str(),
                                 /* deleteIfExists = */ false);
        formulaDb.reset(formulaLevDb);
    } catch (exception& e) {
        PRINT_WARN("%s\n", e.what());
        goto failure;
    }
    meaningDictionary.reset(new MeaningDictionary());

    indexBuilder.reset(new index::IndexBuilder(formulaDb.get(), crawlDb.get(),
                                               &index, meaningDictionary.get(),
                                               indexingOptions));
    for (string it : config.harvestLoadPaths) {
        AbsPath harvestPath(it);
        if (access(it.c_str(), F_OK) != 0) {
            PRINT_WARN("%s: No such file or directory...skipping\n",
                       it.c_str());
            ++invalid_paths;
            continue;
        }
        // do we have read permission?
        if (access(it.c_str(), R_OK) != 0) {
            PRINT_WARN("%s: Permission denied.\n", it.c_str());
            ++invalid_paths;
            continue;
        }
        printf("Loading from %s...\n", it.c_str());
        printf("%d expressions loaded.\n",
               parser::loadMwsHarvestFromDirectory(indexBuilder.get(),
                                                   harvestPath,
                                                   config.harvestFileExtension,
                                                   config.recursive));
        fflush(stdout);
    }

    // Were all paths invalid?
    if (invalid_paths == config.harvestLoadPaths.size()) {
        PRINT_WARN("All paths were invalid. Aborting...\n");
        goto failure;
    }
    memsector_create(&mwsr, (output_dir + "/" + INDEX_MEMSECTOR_FILE).c_str());
    index.exportToMemsector(&mwsr);
    PRINT_LOG("Created index of %s\n",
              humanReadableByteCount(mwsr.ms.index_size,
                                     /*si=*/false).c_str());

    fb.open((output_dir + "/" + MEANING_DICTIONARY_FILE).c_str(),
            std::ios::out);
    meaningDictionary->save(os);
    fb.close();

    clearxmlparser();

    PRINT_LOG("Package saved to %s, total size: %s\n", output_dir.c_str(),
              humanReadableByteCount(getDirectorySize(output_dir),
                                     /*si=*/false).c_str());

    return EXIT_SUCCESS;

failure:
    clearxmlparser();
    return EXIT_FAILURE;
}

}  // namespace index
}  // namespace mws

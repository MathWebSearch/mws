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
  * @file mws-index.cpp
  * @brief mws-index executable
  * @author Radu Hambasan
  * @date 18 Jan 2014
  */

#include <stdexcept>
using std::exception;
#include <string>
using std::string;
#include <fstream>

#include "common/utils/FlagParser.hpp"
using common::utils::FlagParser;
#include "mws/dbc/LevCrawlDb.hpp"
#include "mws/dbc/LevFormulaDb.hpp"
#include "mws/index/MwsIndexNode.hpp"
#include "mws/index/memsector.h"
#include "mws/index/MeaningDictionary.hpp"
using mws::index::MeaningDictionary;
#include "mws/xmlparser/loadMwsHarvestFromFd.hpp"

#include "build-gen/config.h"

const uint32_t DEFAULT_MEMSECTOR_SIZE = (500 * 1024 * 1024);

using namespace mws;

int main(int argc, char* argv[]) {
    string output_dir;
    memsector_writer_t mwsr;
    string harvest_path;
    int ret;
    int64_t memsector_size;

    dbc::CrawlDb*             crawlDb;
    dbc::FormulaDb*           formulaDb;
    MwsIndexNode*             data;
    MeaningDictionary* meaningDictionary;
    index::IndexManager*      indexManager;
    index::IndexingOptions    indexingOptions;
    std::filebuf              fb;
    std::ostream os(&fb);

    FlagParser::addFlag('o', "output-directory",        FLAG_REQ, ARG_REQ);
    FlagParser::addFlag('I', "include-harvest-path",    FLAG_REQ, ARG_REQ);
    FlagParser::addFlag('r', "recursive",               FLAG_OPT, ARG_NONE);
    FlagParser::addFlag('s', "memsector-size",          FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('e', "harvest-file-extension",  FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('c', "enable-ci-renaming",   FLAG_OPT, ARG_NONE);

    string harvestExtension = "harvest";
    if (FlagParser::hasArg('e')) {
        harvestExtension = FlagParser::getArg('e');
    }

    bool recursive = FlagParser::hasArg('r');

    if ((ret = FlagParser::parse(argc, argv)) != 0) {
        fprintf(stderr, "%s", FlagParser::getUsage().c_str());
        goto failure;
    }

    harvest_path = FlagParser::getArg('I');
    output_dir   = FlagParser::getArg('o');
    indexingOptions.renameCi = FlagParser::hasArg('c');
    if (FlagParser::hasArg('s')) {
        memsector_size = atoi(FlagParser::getArg('s').c_str());
    } else {
        memsector_size = DEFAULT_MEMSECTOR_SIZE;
    }

    // if the path exists
    if (access(output_dir.c_str(), 0) == 0) {
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
        dbc::LevCrawlDb* crawlLevDb = new dbc::LevCrawlDb();
        crawlLevDb->create_new((output_dir + "/crawl.db").c_str(),
                               /* deleteIfExists = */ false);
        crawlDb = crawlLevDb;
        dbc::LevFormulaDb* formulaLevDb = new dbc::LevFormulaDb();
        formulaLevDb->create_new((output_dir + "/formula.db").c_str(),
                                 /* deleteIfExists = */ false);
        formulaDb = formulaLevDb;
    } catch (exception& e) {
        PRINT_WARN("%s\n", e.what());
        goto failure;
    }

    data = new MwsIndexNode();
    meaningDictionary = new MeaningDictionary();

    indexManager = new index::IndexManager(formulaDb,
                                           crawlDb,
                                           data,
                                           meaningDictionary,
                                           indexingOptions);

    parser::loadMwsHarvestFromDirectory(indexManager, AbsPath(harvest_path),
                                        harvestExtension, recursive);
    memsector_create(&mwsr,
                     (output_dir + "/memsector.dat").c_str(),
                     memsector_size);

    data->exportToMemsector(&mwsr);
    memsector_save(&mwsr);

    fb.open((output_dir + "/meaning.dat").c_str(), std::ios::out);
    meaningDictionary->save(os);
    fb.close();

    delete crawlDb;
    delete formulaDb;
    delete data;
    delete meaningDictionary;
    delete indexManager;

    return EXIT_SUCCESS;

failure:
    return EXIT_FAILURE;
}


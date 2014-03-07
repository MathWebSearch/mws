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
  *
  * This executable build a math web search index and saves it.
  */

#include <string>
#include <fstream>

#include "types/MeaningDictionary.hpp"
#include "xmlparser/loadMwsHarvestFromFd.hpp"
#include "dbc/LevCrawlDb.hpp"
#include "dbc/LevFormulaDb.hpp"
#include "dbc/MemCrawlDb.hpp"
#include "dbc/MemFormulaDb.hpp"
#include "index/MwsIndexNode.hpp"
#include "common/utils/FlagParser.hpp"
#include "index/memsector.h"
#include "./config.h"

using std::string;
using common::utils::FlagParser;
using namespace mws;

#define DEFAULT_MEMSECTOR_SIZE  (500 * 1024 * 1024)

int main(int argc, char* argv[]) {
    string output_dir;
    memsector_writer_t mwsr;
    string harvest_path;
    int ret;
    int64_t memsector_size;

    dbc::CrawlDb*             crawlDb;
    dbc::FormulaDb*           formulaDb;
    dbc::LevCrawlDb*          crdb;
    dbc::LevFormulaDb*        fmdb;
    MwsIndexNode*             data;
    types::MeaningDictionary* meaningDictionary;
    index::IndexManager*      indexManager;
    std::filebuf              fb;
    std::ostream os(&fb);

    FlagParser::addFlag('d', "output-directory",        FLAG_REQ, ARG_REQ);
    FlagParser::addFlag('I', "include-harvest-path",    FLAG_REQ, ARG_REQ);
    FlagParser::addFlag('r', "recursive",               FLAG_OPT, ARG_NONE);
    FlagParser::addFlag('s', "memsector-size",          FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('e', "harvest-file-extension",  FLAG_OPT, ARG_REQ);

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
    output_dir   = FlagParser::getArg('d');

    if (FlagParser::hasArg('s')) {
        memsector_size = atoi(FlagParser::getArg('s').c_str());
    } else {
        memsector_size = DEFAULT_MEMSECTOR_SIZE;
    }

    crdb = new dbc::LevCrawlDb();
    if (crdb->create_new((output_dir + "crawl.db").c_str()) == -1) {
        fprintf(stderr, "error while creating crawl database\n");
        goto failure;
    }

    crawlDb = crdb;

    fmdb = new dbc::LevFormulaDb();
    if (fmdb->create_new((output_dir + "formula.db").c_str()) == -1) {
        fprintf(stderr, "error while creating formula database\n");
        goto failure;
    }

    formulaDb = fmdb;


    data = new MwsIndexNode();
    meaningDictionary = new types::MeaningDictionary();

    indexManager = new index::IndexManager(formulaDb,
                                           crawlDb,
                                           data,
                                           meaningDictionary);

    parser::loadMwsHarvestFromDirectory(indexManager, AbsPath(harvest_path),
                                        harvestExtension, recursive);
    memsector_create(&mwsr,
                     (output_dir + "memsector.dat").c_str(),
                     memsector_size);

    data->exportToMemsector(&mwsr);
    memsector_save(&mwsr);

    fb.open((output_dir + "meaning.dat").c_str(), std::ios::out);

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


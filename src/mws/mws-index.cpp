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

#include <cinttypes>
#include <stdexcept>
using std::exception;
#include <string>
using std::string;
#include <fstream>

#include "common/utils/FlagParser.hpp"
using common::utils::FlagParser;
#include "mws/dbc/LevCrawlDb.hpp"
#include "mws/dbc/LevFormulaDb.hpp"
#include "mws/index/TmpIndex.hpp"
using mws::index::TmpIndex;
#include "mws/index/memsector.h"
#include "mws/index/MeaningDictionary.hpp"
using mws::index::MeaningDictionary;
#include "mws/index/IndexBuilder.hpp"
using mws::index::loadHarvests;
#include "mws/index/IndexWriter.hpp"
using mws::index::IndexConfiguration;
using mws::index::createCompressedIndex;
#include "mws/xmlparser/clearxmlparser.hpp"
using mws::clearxmlparser;
#include "build-gen/config.h"

using namespace mws;

int main(int argc, char* argv[]) {
    IndexConfiguration indexConfig;

    FlagParser::addFlag('o', "output-directory", FLAG_REQ, ARG_REQ);
    FlagParser::addFlag('I', "include-harvest-path", FLAG_REQ, ARG_REQ);
    FlagParser::addFlag('r', "recursive", FLAG_OPT, ARG_NONE);
    FlagParser::addFlag('e', "harvest-file-extension", FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('c', "enable-ci-renaming", FLAG_OPT, ARG_NONE);

    if (FlagParser::parse(argc, argv) != 0) {
        fprintf(stderr, "%s", FlagParser::getUsage().c_str());
        return EXIT_FAILURE;
    }

    indexConfig.harvester.fileExtension = "harvest";
    if (FlagParser::hasArg('e')) {
        indexConfig.harvester.fileExtension = FlagParser::getArg('e');
    }

    indexConfig.harvester.recursive = FlagParser::hasArg('r');

    indexConfig.harvester.paths = FlagParser::getArgs('I');
    indexConfig.dataPath = FlagParser::getArg('o');
    indexConfig.encoding.renameCi = FlagParser::hasArg('c');

    return createCompressedIndex(indexConfig);
}

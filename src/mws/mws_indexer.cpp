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
  * @brief MathWebSearch index builder
  * @file mws_indexer.cpp
  * @author Corneliu-Claudiu Prodescu
  * @date 22 May 2013
  *
  * License: GPL v3
  *
  */

// System includes

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <unistd.h>
#include <string>
#include <fstream>
#include <ostream>

// Local includes

#include "common/utils/FlagParser.hpp"
#include "common/utils/macro_func.h"
#include "mws/index/MwsIndexNode.hpp"
#include "mws/index/memsector.h"
#include "mws/xmlparser/loadMwsHarvestFromFd.hpp"

using namespace std;
using namespace mws;

int main(int argc, char* argv[]) {
    int ret;
    memsector_writer_t mswr;
    string ms_path;
    string dict_path;

    // Parsing the flags
    FlagParser::addFlag('I', "include-harvest-path", FLAG_REQ, ARG_REQ );
    FlagParser::addFlag('D', "data-path",            FLAG_REQ, ARG_REQ );

    if ((ret = FlagParser::parse(argc, argv))
            != 0)
    {
        fprintf(stderr, "%s", FlagParser::getUsage().c_str());
        return EXIT_FAILURE;
    }

    // harvest paths
    vector<string> harvestLoadPaths = FlagParser::getArgs('I');
    // data-path
    string data_path = FlagParser::getArg('D');

    MwsIndexNode* index = new MwsIndexNode();
    MeaningDictionary dict;
    PageDbHandle db_handle;
    FAIL_ON(db_handle.init(data_path) != 0);

    IndexContext ictxt;
    ictxt.root = index;
    ictxt.page_data_db = &db_handle;
    ictxt.meaning_id_dict = &dict;

    for (vector<string>::iterator it = harvestLoadPaths.begin();
         it != harvestLoadPaths.end(); it++) {
        AbsPath harvestPath(*it);
        printf("Loading from %s...\n", it->c_str());
        printf("%d expressions loaded.\n",
               loadMwsHarvestFromDirectory(harvestPath, &ictxt));
    }

    ms_path = data_path + "/index.bin";
    FAIL_ON(memsector_create(&mswr, ms_path.c_str(), 100 * 1024 * 1024) != 0);
    printf("Memsector %s created\n", ms_path.c_str());

    index->exportToMemsector(&mswr);
    printf("Index exported to memsector\n");
    printf("Space used: %d Kb\n",
           memsector_size_inuse(&mswr.ms_header->alloc_header) / 1024);
    FAIL_ON(memsector_save(&mswr) != 0);

    {
        dict_path = data_path + "/meaning_dictionary.txt";
        filebuf fb;
        fb.open(dict_path.c_str(), std::ios::out);
        ostream os(&fb);
        dict.save(os);
    }

    delete index;
    ictxt.page_data_db->close();

    return EXIT_SUCCESS;

fail:
    return EXIT_FAILURE;
}

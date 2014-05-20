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
  * @brief This takes harvest files and delivers json files for elasticsearch
  *
  * @file harvests2json
  * @author Radu hambasan
  * @date 04 May 2014
  *
  * License: GPL v3
  *
  */

// System includes

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <json.h>

#include <algorithm>
#include <string>
#include <vector>
#include <utility>
#include <fstream>
using std::filebuf;

#include "common/utils/compiler_defs.h"
#include "common/utils/Path.hpp"
#include "common/utils/util.hpp"
#include "common/utils/FlagParser.hpp"
using common::utils::FlagParser;
#include "mws/index/index.h"
#include "mws/index/IndexManager.hpp"
using mws::index::IndexingOptions;
using mws::index::MeaningDictionary;
#include "mws/index/IndexAccessor.hpp"
using mws::index::IndexAccessor;
#include "mws/daemon/Daemon.hpp"
#include "mws/dbc/LevFormulaDb.hpp"
using mws::dbc::FormulaDb;
using mws::dbc::LevFormulaDb;

#include "mws/xmlparser/processMwsHarvest.hpp"
using mws::parser::Hit;
using mws::parser::ParseResult;
using mws::parser::parseMwsHarvestFromFd;

// Namespaces

using namespace std;

static
int writeHitsToJSON(const string& data, const vector<Hit*>& hits,
                    const string& path) {
    FILE* file = fopen((path + ".json").c_str(), "w");
    json_object *json_doc, *ids, *id_mappings;
    json_doc = json_object_new_object();
    ids = json_object_new_array();
    id_mappings = json_object_new_array();

    for (Hit* hit : hits) {
        json_object_array_add(ids, json_object_new_int(hit->id));
        json_object *id_mapping = json_object_new_object();
        json_object_object_add(id_mapping, "id", json_object_new_int(hit->id));
        json_object_object_add(id_mapping, "xpath",
                               json_object_new_string(hit->xpath.c_str()));
        json_object_object_add(id_mapping, "url",
                               json_object_new_string((path + hit->uri).c_str()));
        json_object_array_add(id_mappings, id_mapping);
    }
    json_object_object_add(json_doc, "ids", ids);
    json_object_object_add(json_doc, "id_mappings", id_mappings);
    json_object_object_add(json_doc, "xhtml",
                           json_object_new_string_len(data.data(),
                                                      data.size()));

    string json_string =
                    json_object_to_json_string(json_doc);

    const char* content = json_string.c_str();
    size_t content_size = json_string.size();
    size_t bytes_written = 0;

    while (bytes_written < content_size) {
        bytes_written += fwrite(content + bytes_written,
                               sizeof(char),
                               content_size - bytes_written,
                               file);
    }
    json_object_put(json_doc);
    return content_size;
}

int main(int argc, char* argv[]) {
    mws::daemon::Config config;
    FlagParser::addFlag('I', "index-path",              FLAG_REQ, ARG_REQ);
    FlagParser::addFlag('H', "harvest-path",            FLAG_REQ, ARG_REQ);
    FlagParser::addFlag('c', "enable-ci-renaming",      FLAG_OPT, ARG_NONE);
    FlagParser::addFlag('e', "harvest-file-extension",  FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('r', "recursive",               FLAG_OPT, ARG_NONE);

    if ((FlagParser::parse(argc, argv)) != 0) {
        fprintf(stderr, "%s", FlagParser::getUsage().c_str());
        return EXIT_FAILURE;
    }

    // harvest path
    config.dataPath = FlagParser::getArg('H').c_str();
    config.indexingOptions.renameCi = FlagParser::hasArg('c');
    config.recursive = FlagParser::hasArg('r');
    string indexPath = FlagParser::getArg('I').c_str();

    // harvest file extension
    if (FlagParser::hasArg('e')) {
        config.harvestFileExtension = FlagParser::getArg('e');
    } else {
        config.harvestFileExtension = DEFAULT_MWS_HARVEST_SUFFIX;
    }

    index_handle_t index;
    MeaningDictionary* meaningDictionary;
    FormulaDb* formulaDb;
    LevFormulaDb* fmdb = new LevFormulaDb();
    string fmdbPath = indexPath + "/formula.db";

    try {
        fmdb->open(fmdbPath.c_str());
        formulaDb = fmdb;
    }
    catch(const exception &e) {
        PRINT_WARN("Initializing database: %s\n", e.what());
        return EXIT_FAILURE;
    }

    /*
     * Initializing data
     */
    string ms_path = indexPath + "/memsector.dat";
    memsector_handle_t msHandle;
    memsector_load(&msHandle, ms_path.c_str());

    index.ms = msHandle.ms;
    index.root = memsector_get_root(&msHandle);

    /*
     * Initializing meaningDictionary
     */
    meaningDictionary = new MeaningDictionary();
    filebuf fb;
    istream os(&fb);
    fb.open((indexPath + "/meaning.dat").c_str(), ios::in);
    meaningDictionary->load(os);
    fb.close();

    common::utils::FileCallback fileCallback =
            [&] (const std::string& path, const std::string& prefix) {
        UNUSED(prefix);
        if (common::utils::hasSuffix(path, config.harvestFileExtension)) {
            printf("Loading %s... ", path.c_str());
            int fd = open(path.c_str(), O_RDONLY);
            if (fd < 0) {
                return -1;
            }
            ParseResult parseReturn = parseMwsHarvestFromFd(config, &index,
                                                            meaningDictionary,
                                                            formulaDb, fd);
            writeHitsToJSON(parseReturn.data, parseReturn.hits, path);
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
    if (config.recursive) {
        if (common::utils::foreachEntryInDirectory(config.dataPath,
                                                   fileCallback,
                                                   shouldRecurse)) {
            fprintf(stderr, "Error in foreachEntryInDirectory (recursive)");
            return EXIT_FAILURE;
        }
    } else {
        if (common::utils::foreachEntryInDirectory(config.dataPath,
                                                       fileCallback)) {
            fprintf(stderr, "Error in foreachEntryInDirectory (non-recursive)");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

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
  * @edit Radu Hambasan
  * @date 03 Jun 2014
  * License: GPL v3
  *
  */

#include <libxml/parser.h>
#include <libxml/tree.h>

// System includes

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <json.h>
#include <errno.h>

#include <cassert>
#include <fstream>
#include <utility>
#include <algorithm>
#include <string>
#include <vector>
#include <map>

using std::filebuf;

#include "common/utils/compiler_defs.h"
#include "common/utils/Path.hpp"
#include "common/utils/util.hpp"
#include "common/utils/FlagParser.hpp"
#include "crawler/parser/XmlParser.hpp"
using crawler::parser::parseDocument;
using crawler::parser::getTextByXpath;
using crawler::parser::processXpathResults;
using crawler::parser::getTextFromNode;
using crawler::parser::getXmlFromNode;
using common::utils::FlagParser;
#include "mws/index/index.h"
#include "mws/index/IndexLoader.hpp"
using mws::index::IndexLoader;
using mws::index::LoadingOptions;
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

const char METADATA_XPATH[] = "//metadata/*";
const char TEXT_XPATH[] = "//text";
const char ID_XPATH[] = "//id";
const char MATH_XPATH[] = "//math";

struct DataItems {
    string id;
    map<string, string> metadata;
    string text;
    map<string, string> exprXml;
};

static DataItems* getDataItems(const string& content) {
    xmlDocPtr doc = parseDocument(content.c_str(), content.size());
    DataItems* dataItems = new DataItems();
    dataItems->text = getTextByXpath(doc, TEXT_XPATH);
    dataItems->id = getTextByXpath(doc, ID_XPATH);

    // get math node mappings
    processXpathResults(doc, MATH_XPATH, [&](xmlNode* mathNode) {
        const xmlChar* xmlId = xmlGetProp(mathNode, BAD_CAST "local_id");
        assert(xmlId != nullptr);

        string id((char*)xmlId);
        xmlFree((void*)xmlId);

        dataItems->exprXml[id] = getTextFromNode(mathNode, nullptr);
    });

    // get metadata
    processXpathResults(doc, METADATA_XPATH, [&](xmlNode* metadataItem) {
        string tag((const char*)metadataItem->name);
        dataItems->metadata[tag] = getTextFromNode(metadataItem);
    });

    xmlFreeDoc(doc);
    return dataItems;
}

static void writeParseResultToFile(const ParseResult& parseResult, FILE* file) {
    DataItems* dataItems = getDataItems(parseResult.data);
    json_object* curr_doc, *metadata, *mws_ids, *id_mappings, *mathExprs;

    curr_doc = json_object_new_object();
    mws_ids = json_object_new_array();
    id_mappings = json_object_new_object();
    metadata = json_object_new_object();
    mathExprs = json_object_new_object();

    for (auto& kv : dataItems->metadata) {
        json_object_object_add(metadata, kv.first.c_str(),
                               json_object_new_string(kv.second.c_str()));
    }
    json_object_object_add(curr_doc, "metadata", metadata);

    // create JSON for mws_ids and mws_id
    for (const auto& kv : parseResult.idMappings) {
        json_object_array_add(mws_ids, json_object_new_int(kv.first));
        json_object* id_mapping = json_object_new_object();
        for (const Hit* hit : kv.second) {
            json_object* hitJson = json_object_new_object();
            json_object_object_add(hitJson, "xpath",
                                   json_object_new_string(hit->xpath.c_str()));
            json_object_object_add(id_mapping, hit->uri.c_str(), hitJson);
        }
        json_object_object_add(id_mappings, to_string(kv.first).c_str(),
                               id_mapping);
    }

    json_object_object_add(curr_doc, "mws_ids", mws_ids);
    json_object_object_add(curr_doc, "mws_id", id_mappings);

    for (const auto& kv : dataItems->exprXml) {
        json_object_object_add(mathExprs, kv.first.c_str(),
                               json_object_new_string(kv.second.c_str()));
    }
    json_object_object_add(curr_doc, "math", mathExprs);
    json_object_object_add(curr_doc, "text",
                           json_object_new_string(dataItems->text.c_str()));

    fprintf(file, "{\"index\" : { \"_id\" : \"%s\" } }\n",
            dataItems->id.c_str());
    fputs(json_object_to_json_string(curr_doc), file);
    fprintf(file, "\n");
    json_object_put(curr_doc);

    delete dataItems;
}

int main(int argc, char* argv[]) {
    mws::daemon::Config config;
    FlagParser::addFlag('I', "index-path", FLAG_REQ, ARG_REQ);
    FlagParser::addFlag('H', "harvest-path", FLAG_REQ, ARG_REQ);
    FlagParser::addFlag('c', "enable-ci-renaming", FLAG_OPT, ARG_NONE);
    FlagParser::addFlag('e', "harvest-file-extension", FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('r', "recursive", FLAG_OPT, ARG_NONE);

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

    try {
        LoadingOptions loadingOptions;
        loadingOptions.includeHits = false;
        IndexLoader data(indexPath, loadingOptions);

        common::utils::FileCallback fileCallback = [&](
            const std::string& path, const std::string& prefix) {
            UNUSED(prefix);
            if (common::utils::hasSuffix(path, config.harvestFileExtension)) {
                string outputName = path + ".json";
                FILE* output = fopen(outputName.c_str(), "wx");
                if (output == nullptr) {
                    if (errno == EEXIST) {
                        PRINT_LOG("Skipping \"%s\": already converted\n",
                                  path.c_str());
                        return 0;
                    } else {
                        PRINT_WARN("Open %s: %s\n", outputName.c_str(),
                                   strerror(errno));
                        return -1;
                    }
                }

                int fd = open(path.c_str(), O_RDONLY);
                if (fd < 0) {
                    PRINT_WARN("Open %s: %s\n", path.c_str(), strerror(errno));
                    fclose(output);
                    return -1;
                }

                vector<ParseResult*> parseResults =
                    parseMwsHarvestFromFd(config, data.getIndexHandle(),
                                          data.getMeaningDictionary(), fd);
                for (const ParseResult* parseResult : parseResults) {
                    writeParseResultToFile(*parseResult, output);
                    delete parseResult;
                }

                close(fd);
                fclose(output);
                PRINT_LOG("Processed %s\n", path.c_str());
            } else {
                PRINT_LOG("Skipping \"%s\": bad extension\n", path.c_str());
            }

            return 0;
        };
        common::utils::DirectoryCallback shouldRecurse = [](
            const std::string partialPath) {
            UNUSED(partialPath);
            return true;
        };

        printf("Processing harvest files...\n");
        if (config.recursive) {
            if (common::utils::foreachEntryInDirectory(
                    config.dataPath, fileCallback, shouldRecurse)) {
                PRINT_WARN("Error in foreachEntryInDirectory (recursive)");
                return EXIT_FAILURE;
            }
        } else {
            if (common::utils::foreachEntryInDirectory(config.dataPath,
                                                       fileCallback)) {
                PRINT_WARN("Error in foreachEntryInDirectory (non-recursive)");
                return EXIT_FAILURE;
            }
        }
    } catch (exception& e) {
        PRINT_WARN("%s\n", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

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

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <json.h>
#include <errno.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include <map>
using std::map;
#include <string>
using std::string;
using std::to_string;
#include <vector>
using std::vector;
#include <stdexcept>
using std::exception;

#include "build-gen/config.h"
#include "common/utils/compiler_defs.h"
#include "common/utils/Path.hpp"
#include "common/utils/util.hpp"
using common::utils::hasSuffix;
#include "common/utils/FlagParser.hpp"
using common::utils::FlagParser;
#include "crawler/parser/XmlParser.hpp"
using crawler::parser::parseDocument;
using crawler::parser::getTextByXpath;
using crawler::parser::processXpathResults;
using crawler::parser::getTextFromNode;
using crawler::parser::getXmlFromNode;
#include "mws/xmlparser/processMwsHarvest.hpp"
using mws::parser::HarvestProcessor;
#include "mws/index/index.h"
#include "mws/index/MeaningDictionary.hpp"
using mws::index::MeaningDictionary;
#include "mws/index/IndexLoader.hpp"
using mws::index::IndexLoader;
using mws::index::LoadingOptions;
#include "mws/index/IndexWriter.hpp"
using mws::index::HarvesterConfiguration;
#include "mws/index/IndexAccessor.hpp"
using mws::index::IndexAccessor;
#include "mws/index/ExpressionEncoder.hpp"
using mws::index::HarvestEncoder;
using mws::index::ExpressionEncoder;
#include "mws/types/CmmlToken.hpp"
using mws::types::CmmlToken;
using mws::types::TokenCallback;
#include "mws/types/MwsAnswset.hpp"
using mws::MwsAnswset;
#include "mws/types/Query.hpp"
using mws::types::Query;
#include "mws/dbc/LevFormulaDb.hpp"
using mws::dbc::FormulaDb;
using mws::dbc::LevFormulaDb;
#include "mws/dbc/CrawlDb.hpp"
using mws::dbc::CrawlId;
#include "mws/query/SearchContext.hpp"
using mws::query::SearchContext;
#include "mws/types/FormulaPath.hpp"
using mws::types::FormulaId;

const char METADATA_XPATH[] = "//metadata/*";
const char TEXT_XPATH[] = "//text";
const char ID_XPATH[] = "//id";
const char MATH_XPATH[] = "//math";

struct Hit {
    std::string uri;
    std::string xpath;
};

struct ParseResult {
    std::string data;
    std::map<mws::types::FormulaId, std::vector<Hit*>> idMappings;

    ~ParseResult() {
        for (auto& kv : idMappings) {
            for (auto& hit : kv.second) {
                delete hit;
            }
        }
    }
};

/**
 * @brief parseMwsHarvestFromFd Analyze a harvest, given a complete index
 * @param config extra configuration options
 * @param index the loaded index
 * @param meaningDictionary
 * @param fd file descriptor of the harvest file. The caller is responsable
 * for closing it.
 * @return a vector, where every element is a ParseResult associated with a doc
 */
static vector<ParseResult*> parseMwsHarvestFromFd(
    const ExpressionEncoder::Config& encodingConfig,
    IndexAccessor::Index* index, MeaningDictionary* meaningDictionary, int fd);

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
    struct Config {
        string indexPath;
        HarvesterConfiguration harvester;
        ExpressionEncoder::Config encoding;
    } config;

    FlagParser::addFlag('I', "index-path", FLAG_REQ, ARG_REQ);
    FlagParser::addFlag('H', "harvest-path", FLAG_REQ, ARG_REQ);
    FlagParser::addFlag('c', "enable-ci-renaming", FLAG_OPT, ARG_NONE);
    FlagParser::addFlag('e', "harvest-file-extension", FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('r', "recursive", FLAG_OPT, ARG_NONE);

    if ((FlagParser::parse(argc, argv)) != 0) {
        fprintf(stderr, "%s", FlagParser::getUsage().c_str());
        return EXIT_FAILURE;
    }

    config.harvester.paths = {FlagParser::getArg('H').c_str()};
    config.harvester.recursive = FlagParser::hasArg('r');
    config.indexPath = FlagParser::getArg('I').c_str();
    config.encoding.renameCi = FlagParser::hasArg('c');

    if (FlagParser::hasArg('e')) {
        config.harvester.fileExtension = FlagParser::getArg('e');
    } else {
        config.harvester.fileExtension = DEFAULT_MWS_HARVEST_SUFFIX;
    }

    try {
        LoadingOptions loadingOptions;
        loadingOptions.includeHits = false;
        IndexLoader data(config.indexPath, loadingOptions);

        common::utils::FileCallback fileCallback = [&](
            const std::string& path, const std::string& prefix) {
            UNUSED(prefix);
            if (hasSuffix(path, config.harvester.fileExtension)) {
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

                vector<ParseResult*> parseResults = parseMwsHarvestFromFd(
                    config.encoding, data.getIndexHandle(),
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
        common::utils::DirectoryCallback shouldRecurse = [&](
            const std::string partialPath) {
            UNUSED(partialPath);
            return config.harvester.recursive;
        };

        for (string harvestPath : config.harvester.paths) {
            printf("Processing harvest files %s...\n", harvestPath.c_str());
            if (common::utils::foreachEntryInDirectory(
                    harvestPath, fileCallback, shouldRecurse)) {
                PRINT_WARN("Error in foreachEntryInDirectory");
                return EXIT_FAILURE;
            }
        }
    } catch (exception& e) {
        PRINT_WARN("%s\n", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

class HarvestParser : public HarvestProcessor {
 public:
    int processExpression(const CmmlToken* tok, const string& exprUri,
                          const uint32_t& crawlId);
    CrawlId processData(const string& data);
    vector<ParseResult*> getParseResults();
    HarvestParser(IndexAccessor::Index* index,
                  MeaningDictionary* meaningictionary,
                  ExpressionEncoder::Config indexingOptions);

 private:
    IndexAccessor::Index* _index;
    MeaningDictionary* _meaningDictionary;
    ExpressionEncoder::Config _indexingOptions;
    CrawlId _idCounter;
    Query::Options _queryOptions;
    /* each CrawlId uniquely identifies a document */
    map<CrawlId, ParseResult*> documents;
};

HarvestParser::HarvestParser(IndexAccessor::Index* index,
                             MeaningDictionary* meaningDictionary,
                             ExpressionEncoder::Config indexingOptions)
    : _index(index),
      _meaningDictionary(meaningDictionary),
      _indexingOptions(std::move(indexingOptions)),
      _idCounter(0) {
    _queryOptions.includeHits = false;
}

vector<ParseResult*> HarvestParser::getParseResults() {
    vector<ParseResult*> results;
    results.reserve(documents.size());

    for (auto& kv : documents) {
        results.push_back(kv.second);
    }

    return results;
}

int HarvestParser::processExpression(const CmmlToken* expression,
                                     const string& exprUri,
                                     const uint32_t& crawlId) {
    assert(expression != nullptr);
    assert(crawlId <= _idCounter);

    HarvestEncoder encoder(_meaningDictionary);
    u_int32_t numSubExpressions = 0;

    expression->foreachSubexpression([&](const CmmlToken* subexpression) {
        vector<encoded_token_t> encodedFormula;
        encoder.encode(_indexingOptions, subexpression, &encodedFormula,
                       nullptr);

        MwsAnswset* result;
        SearchContext ctxt(encodedFormula, _queryOptions);
        result = ctxt.getResult<IndexAccessor>(_index,
                                               /* dbQueryManager = */ nullptr,
                                               /* offset = */ 0,
                                               /* size = */ 1,
                                               /* maxTotal = */ 1);
        numSubExpressions += result->total;
        auto hit = new Hit();
        FormulaId fmId = *(result->ids.begin());
        hit->uri = exprUri;
        hit->xpath = subexpression->getXpath();

        ParseResult* doc = documents[crawlId];
        (doc->idMappings[fmId]).push_back(hit);
        delete result;
    });

    return numSubExpressions;
}

CrawlId HarvestParser::processData(const string& data) {
    documents[++_idCounter] = new ParseResult();
    documents[_idCounter]->data = data;

    return _idCounter;
}

static vector<ParseResult*> parseMwsHarvestFromFd(
    const ExpressionEncoder::Config& encodingConfig,
    IndexAccessor::Index* index, MeaningDictionary* meaningDictionary, int fd) {

    HarvestParser harvestParser(index, meaningDictionary, encodingConfig);
    processHarvestFromFd(fd, &harvestParser);
    return harvestParser.getParseResults();
}

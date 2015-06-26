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
 *  @file docs2harvest.cpp
 *  @author Corneliu C Prodescu <cprodescu@gmail.com>
 *
 *  @edit Radu Hambasan
 *  @date 07 Feb 2014
 *
 *  Harvest HTML and XHTML documents given as command line arguments.
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <json/json.h>
#include <json/json_object.h>

#include <string>
using std::string;
using std::to_string;
#include <stdexcept>
using std::exception;
using std::runtime_error;
#include <vector>
using std::vector;

#include "crawler/parser/MathParser.hpp"
using crawler::parser::Harvest;
using crawler::parser::HarvesterConfiguration;
using crawler::parser::createHarvestFromDocument;
using crawler::parser::cleanupMathParser;
#include "common/utils/compiler_defs.h"
#include "common/utils/FlagParser.hpp"
using common::utils::FlagParser;
#include "common/utils/util.hpp"
using common::utils::getFileContents;
using common::utils::create_directory;

const char DEFAULT_OUTPUT_DIRECTORY[] = ".";

static HarvesterConfiguration readHarvesterConfigurationFile(string path);

int main(int argc, char* argv[]) {
    string outputDirectory = DEFAULT_OUTPUT_DIRECTORY;
    HarvesterConfiguration config;

    FlagParser::addFlag('o', "output-directory", FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('x', "document-uri-xpath", FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('c', "config-file", FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('n', "no-data", FLAG_OPT, ARG_NONE);
    FlagParser::setMinNumParams(0);

    if (FlagParser::parse(argc, argv) != 0) {
        fprintf(stderr, "%s", FlagParser::getUsage().c_str());
        return EXIT_FAILURE;
    }

    if (FlagParser::hasArg('o')) {
        outputDirectory = FlagParser::getArg('o');
        create_directory(outputDirectory);
    }
    if (FlagParser::hasArg('c')) {
        config = readHarvesterConfigurationFile(FlagParser::getArg('c'));
    }
    if (FlagParser::hasArg('x')) {
        config.documentIdXpath = FlagParser::getArg('x');
    }
    if (FlagParser::hasArg('n')) {
        config.shouldSaveData = false;
    }

    PRINT_LOG("Using output directory %s\n", outputDirectory.c_str());
    PRINT_LOG("Using configuration:\n%s", config.toString().c_str());
    atexit(cleanupMathParser);

    string harvestPathTemplate = outputDirectory + "/harvest_XXXXXX.harvest";
    char* harvestPath = strdup(harvestPathTemplate.c_str());
    FILE* harvestOutStream = nullptr;
    int harvestFd = mkstemps(harvestPath, /* suffixlen = */ 8);
    if (harvestFd < 0) {
        perror("mkstemp");
        goto failure;
    }

    harvestOutStream = fdopen(harvestFd, "w");
    if (harvestOutStream == nullptr) {
        perror("fdopen");
        goto failure;
    }

    fputs(
        "<?xml version=\"1.0\" ?>\n"
        "<mws:harvest xmlns:m=\"http://www.w3.org/1998/Math/MathML\"\n"
        "             xmlns:mws=\"http://search.mathweb.org/ns\">\n",
        harvestOutStream);
    {
        const vector<string>& filePaths = FlagParser::getParams();
        int data_id = 0;
        for (const string& filePath : filePaths) {
            try {
                config.data_id = to_string(data_id);
                const Harvest harvest =
                    createHarvestFromDocument(filePath, config);
                fputs(harvest.dataElement.c_str(), harvestOutStream);
                for (const string& element : harvest.contentMathElements) {
                    fputs(element.c_str(), harvestOutStream);
                }
                PRINT_LOG("Parsed %s: %zd math elements\n", filePath.c_str(),
                          harvest.contentMathElements.size());
            } catch (exception& e) {
                PRINT_WARN("Parsing %s: %s\n", filePath.c_str(), e.what());
            }
            data_id++;
        }
    }
    fputs("</mws:harvest>\n", harvestOutStream);
    fclose(harvestOutStream);

    PRINT_LOG("Written harvest %s\n", harvestPath);

    free(harvestPath);
    return EXIT_SUCCESS;

failure:
    if (harvestOutStream) fclose(harvestOutStream);
    free(harvestPath);

    return EXIT_FAILURE;
}

static json_object* getJsonChild(json_object* json_parent, const char* key) {
    json_object* child = json_object_object_get(json_parent, key);
    if (child != nullptr) {
        return child;
    } else {
        PRINT_WARN("Cannot find key \"%s\"\n", key);
        return nullptr;
    }
}

static bool getJsonBool(json_object* json_parent, const char* key) {
    bool value = false;
    json_object* json_bool_object = getJsonChild(json_parent, key);
    if (json_bool_object != nullptr) {
        json_type type = json_object_get_type(json_bool_object);
        if (type != json_type_boolean) {
            PRINT_WARN("Value of \"%s\" should be boolean\n", key);
        } else {
            value = (json_object_get_boolean(json_bool_object) == TRUE);
        }
    }

    return value;
}

static string getJsonString(json_object* json_parent, const char* key) {
    string value;
    json_object* json_string_object = getJsonChild(json_parent, key);
    if (json_string_object != nullptr) {
        json_type type = json_object_get_type(json_string_object);
        if (type != json_type_string) {
            PRINT_WARN("metadata : %s should be a string\n", key);
        } else {
            value = json_object_get_string(json_string_object);
        }
    }

    return value;
}

static HarvesterConfiguration readHarvesterConfigurationFile(string path) {
    HarvesterConfiguration config;
    string content = getFileContents(path);
    json_object* jsonConfig = json_tokener_parse(content.c_str());
    if (jsonConfig == nullptr) {
        throw runtime_error("Cannot parse JSON configuration " + path);
    }

#define readBoolConfigProperty(x) config.x = getJsonBool(jsonConfig, #x)
#define readStringConfigProperty(x) config.x = getJsonString(jsonConfig, #x)

    readBoolConfigProperty(shouldSaveData);
    readStringConfigProperty(documentIdXpath);
    readStringConfigProperty(textWithMathXpath);

#undef readBoolConfigProperty
#undef readStringConfigProperty

    json_object* jsonMetadata = getJsonChild(jsonConfig, "metadata");
    if (jsonMetadata != nullptr) {
        json_object_object_foreach(jsonMetadata, key, value) {
            HarvesterConfiguration::MetadataItem item;
            UNUSED(value);
            item.name = key;
            item.xpath = getJsonString(jsonMetadata, key);
            config.metadataItems.push_back(item);
        }
    }
    json_object_put(jsonConfig);

    return config;
}

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
 *  @edited Radu Hambasan
 *  @date 07 Feb 2014
 *  Harvest HTML and XHTML documents given as command line arguments.
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <stdexcept>
using std::exception;
#include <string>
using std::string;
#include <vector>
using std::vector;

#include "crawler/parser/MathParser.hpp"
using crawler::parser::Harvest;
using crawler::parser::getHarvestFromDocument;
using crawler::parser::cleanupMathParser;

#include "common/utils/compiler_defs.h"
#include "common/utils/FlagParser.hpp"
using common::utils::FlagParser;
#include "common/utils/util.hpp"
using common::utils::getFileContents;

const char DEFAULT_OUTPUT_DIRECTORY[] = ".";
const char DEFAULT_DOCUMENT_URI_XPATH[] =
        "//*[local-name()='span' and @class='number']";

int main(int argc, char *argv[]) {
    string outputDirectory = DEFAULT_OUTPUT_DIRECTORY;
    string documentUriXpath = DEFAULT_DOCUMENT_URI_XPATH;
    bool saveData = true;

    FlagParser::addFlag('o', "output-directory",  FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('x', "document-uri-xpath", FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('n', "no-data", FLAG_OPT, ARG_NONE);
    FlagParser::setMinNumParams(1);

    if (FlagParser::parse(argc, argv) != 0) {
        fprintf(stderr, "%s", FlagParser::getUsage().c_str());
        return EXIT_FAILURE;
    }

    if (FlagParser::hasArg('O')) outputDirectory = FlagParser::getArg('o');
    if (FlagParser::hasArg('x')) documentUriXpath = FlagParser::getArg('x');
    saveData = !FlagParser::hasArg('n');

    PRINT_LOG("Using output directory %s\n", outputDirectory.c_str());
    PRINT_LOG("Using document title xpath %s\n", documentUriXpath.c_str());
    atexit(cleanupMathParser);

    string harvestPathTemplate = outputDirectory + "/harvest_XXXXXX.harvest";
    char* harvestPath = strdup(harvestPathTemplate.c_str());
    FILE* harvestOutStream = NULL;
    int harvestFd = mkstemps(harvestPath, /* suffixlen = */ 8);
    if (harvestFd < 0) {
        perror("mkstemp");
        goto failure;
    }

    harvestOutStream = fdopen(harvestFd, "w");
    if (harvestOutStream == NULL) {
        perror("fdopen");
        goto failure;
    }

    fputs("<?xml version=\"1.0\" ?>\n"
          "<mws:harvest xmlns:m=\"http://www.w3.org/1998/Math/MathML\"\n"
          "             xmlns:mws=\"http://search.mathweb.org/ns\">\n",
          harvestOutStream);
    {
        const vector<string>& files = FlagParser::getParams();
        int data_id = 0;
        for (const string& file : files) {
            try {
                const string content = getFileContents(file.c_str());
                Harvest harvest = getHarvestFromDocument(
                            content, data_id, documentUriXpath, saveData);
                fputs(harvest.dataElement.c_str(), harvestOutStream);
                for (const string& element : harvest.contentMathElements) {
                    fputs(element.c_str(), harvestOutStream);
                }
                PRINT_LOG("Parsed %s: %zd math elements\n",
                          file.c_str(), harvest.contentMathElements.size());
            } catch (exception& e) {
                PRINT_WARN("Parsing %s: %s\n", file.c_str(), e.what());
            }
            data_id++;
        }
    }
    fputs("</mws:harvest>\n", harvestOutStream);
    fclose(harvestOutStream);

    PRINT_LOG("Written harvest %s\n", harvestPath);

    return EXIT_SUCCESS;

failure:
    if (harvestOutStream) fclose(harvestOutStream);

    return EXIT_FAILURE;
}

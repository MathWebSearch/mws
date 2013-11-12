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
 *
 * quick crawl a number of pages given on stdin, one per line
 *
 */

// System includes
#include <string>
#include <vector>
#include <queue>
#include <set>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fstream>

// Local includes
#include "crawler/utils/MwsGetMath.hpp"
#include "crawler/utils/Page.hpp"

#include "common/utils/FlagParser.hpp"

using namespace std;
using namespace mws;

static std::string get_file_contents(const char *filename)
{
    string contents;

    try {
        std::ifstream in(filename, std::ios::in | std::ios::binary);
        if (in)
        {
            in.seekg(0, std::ios::end);
            contents.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(&contents[0], contents.size());
            in.close();
        }
        return contents;
    } catch (...) {
        perror("get_file_contents");
        return "";
    }
}


int main(int argc, char *argv[])
{
    string outdir = ".";
    string root   = ".";
    FILE* harvest = NULL;

    FlagParser::addFlag('O', "outdir",  FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('R', "root",    FLAG_OPT, ARG_REQ);

    if (FlagParser::parse(argc, argv) != 0) {
        fprintf(stderr, "%s", FlagParser::getUsage().c_str());
        return EXIT_FAILURE;
    }

    if (FlagParser::hasArg('O')) outdir = FlagParser::getArg('O');
    if (FlagParser::hasArg('R')) root = FlagParser::getArg('R');


    string harvest_templ_str = outdir + "/harvest_XXXXXX.xml";
    char* harvest_templ = strdup(harvest_templ_str.c_str());
    int harvest_fd = mkstemps(harvest_templ, /* suffixlen = */ 4);
    if (harvest_fd < 0) {
        perror("mkstemp");
        goto failure;
    }

    harvest = fdopen(harvest_fd, "w");
    if (harvest == NULL) {
        perror("fdopen");
        goto failure;
    }

    fputs("<?xml version=\"1.0\" ?>\n"
          "<mws:harvest xmlns:m=\"http://www.w3.org/1998/Math/MathML\"\n"
          "             xmlns:mws=\"http://search.mathweb.org/ns\">\n",
          harvest);
    {
        const vector<string>& params = FlagParser::getParams();
        for (vector<string>::const_iterator it = params.begin(); it != params.end(); it++) {
            string url = root + "/" + *it;
            string content = get_file_contents(it->c_str());
            vector<string> math = get_math_xhtml(content, url);
            for(vector<string>::iterator it = math.begin(); it != math.end() ; it++) {
                fputs(it->c_str(), harvest);
            }
        }
    }
    fputs("</mws:harvest>\n", harvest);

    fclose(harvest);

    return EXIT_SUCCESS;

failure:
    if (harvest) fclose(harvest);
    return EXIT_FAILURE;
}

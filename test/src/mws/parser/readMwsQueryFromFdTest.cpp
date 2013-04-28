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
  * @brief Testing for the readMwsQueryFromFd function - implementation
  *
  * @file readMwsQueryFromFdTest.cpp
  * @author Prodescu Corneliu-Claudiu <c.prodescu@jacobs-university.de>
  * @date 27 Apr 2011
  *
  * License: GPL v3
  *
  */

// System includes

#include <sys/types.h>                 // Primitive System datatypes
#include <sys/stat.h>                  // POSIX File characteristics
#include <fcntl.h>                     // File control operations
#include <libxml/parser.h>             // LibXML parser header
#include <string>                      // C++ String header
#include <stdlib.h>
#include <unistd.h>

// Local includes

#include "mws/xmlparser/readMwsQueryFromFd.hpp"
#include "common/utils/macro_func.h"

#include "config.h"

// Namespaces

using namespace std;
using namespace mws;


int main()
{
    MwsQuery*   result;
    int         fd;
    const char* xmlfile  = "MwsQuery1.xml";
    string      xml_path = (string) MWS_TESTDATA_PATH +
                            "/" + (string) xmlfile;

    fd = open(xml_path.c_str(), O_RDONLY);
    FAIL_ON(fd < 0);

    result = readMwsQueryFromFd(fd);

    FAIL_ON(result == NULL);
    FAIL_ON(result->warnings != 0);
    FAIL_ON(result->attrResultMaxSize != 24);
    FAIL_ON(result->attrResultLimitMin != 1);
    FAIL_ON(result->tokens.size() != (size_t) 1);

    delete result;

    (void) close(fd);

    (void) xmlCleanupParser();

    return EXIT_SUCCESS;

fail:
    return EXIT_FAILURE;
}

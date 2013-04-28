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
  * @brief Testing for the writeXmlAnswsetToFd function - implementation
  *
  * @file writeXmlAnswsetToFd.cpp
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
#include <string>                      // C++ String header
#include <stdlib.h>
#include <unistd.h>

// Local includes

#include "mws/xmlparser/writeXmlAnswsetToFd.hpp"
#include "common/utils/macro_func.h"

// Macros

#define TMP_PATH "/tmp/"

// Namespaces

using namespace std;
using namespace mws;

int main() {
    MwsAnswset* answset;
    MwsAnsw*    answ;
    int         fd;
    int         ret;
    const char* xmlfile  = "MwsAnswset1.xml";
    string      xml_path = (string) TMP_PATH +
                            "/" + (string) xmlfile;
    
    // Building an anserset
    answset = new MwsAnswset;
    answ = new MwsAnsw("www.example.org", "defxml");
    answset->answers.push_back(answ);

    fd = open(xml_path.c_str(), O_WRONLY | O_CREAT,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    FAIL_ON(fd < 0);

    // Running the function
    ret = writeXmlAnswsetToFd(answset, fd);
    FAIL_ON(ret != 0);

    // Freeing the Answer Set
    delete answset;

    (void) close(fd);

    return EXIT_SUCCESS;

fail:
    return EXIT_FAILURE;
}

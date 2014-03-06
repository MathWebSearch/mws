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

#include <sys/types.h>                 // Primitive System datatypes
#include <sys/stat.h>                  // POSIX File characteristics
#include <fcntl.h>                     // File control operations
#include <string>                      // C++ String header
#include <stdlib.h>
#include <unistd.h>

#include "mws/xmlparser/writeXmlAnswsetToFd.hpp"
#include "common/utils/compiler_defs.h"

// Macros

#define TMP_PATH "/tmp/"


int main() {
    using mws::MwsAnswset;
    using std::string;
    using mws::types::Answer;

    const string xml_path = (string) TMP_PATH + "/MwsAnswset1.xml";

    Answer* answer = new Answer();
    answer->data = "lalala";
    answer->uri = "http://foo";
    answer->xpath = "//*[1]";
    MwsAnswset* answset = new MwsAnswset();
    answset->answers.push_back(answer);

    int fd = open(xml_path.c_str(), O_WRONLY | O_CREAT,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    FAIL_ON(fd < 0);
    FAIL_ON(writeXmlAnswsetToFd(answset, fd) != 0);

    delete answset;

    (void) close(fd);

    return EXIT_SUCCESS;

fail:
    return EXIT_FAILURE;
}

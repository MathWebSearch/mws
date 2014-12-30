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

#include <sys/types.h>  // Primitive System datatypes
#include <sys/stat.h>   // POSIX File characteristics
#include <fcntl.h>      // File control operations
#include <string>       // C++ String header
#include <stdlib.h>
#include <unistd.h>

#include "mws/types/Answer.hpp"
#include "mws/types/MwsAnswset.hpp"
#include "mws/xmlparser/XmlResponseFormatter.hpp"
using mws::parser::RESPONSE_FORMATTER_XML;
#include "common/utils/compiler_defs.h"

// Macros

#define TMP_PATH "/tmp/"

int main() {
    using mws::MwsAnswset;
    using std::string;
    using mws::types::Answer;

    const string xml_path = (string)TMP_PATH + "/MwsAnswset1.xml";

    auto answer = new Answer();
    answer->data = "lalala";
    answer->uri = "http://foo";
    answer->xpath = "//*[1]";
    MwsAnswset answset;
    answset.answers.push_back(answer);

    FILE* file = fopen(xml_path.c_str(), "w");
    FAIL_ON(file == nullptr);
    FAIL_ON(RESPONSE_FORMATTER_XML->writeData(&answset, file) != 260);

    (void)fclose(file);

    return EXIT_SUCCESS;

fail:
    return EXIT_FAILURE;
}

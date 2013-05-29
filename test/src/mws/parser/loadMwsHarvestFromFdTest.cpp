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
  * @brief Testing for the loadMwsHarvestFromFd function - implementation
  *
  * @file loadMwsHarvestFromFdTest.cpp
  * @author Prodescu Corneliu-Claudiu <c.prodescu@jacobs-university.de>
  * @date 30 Apr 2011
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

// Local includes

#include "mws/xmlparser/initxmlparser.hpp"
#include "mws/xmlparser/clearxmlparser.hpp"
#include "mws/xmlparser/loadMwsHarvestFromFd.hpp"
#include "mws/dbc/MwsDbConn.hpp"
#include "mws/dbc/PageDbHandle.hpp"
#include "mws/dbc/PageDbConn.hpp"
#include "common/utils/macro_func.h"

#include "config.h"

#define TMPDBENV_PATH   "/tmp"

// Namespaces

using namespace std;
using namespace mws;

int main()
{
    int           fd, ret;
    MeaningDictionary dict;
    PageDbHandle dbhandle;
    MwsIndexNode data;
    IndexContext ictxt;
    ictxt.meaning_id_dict = &dict;
    ictxt.root = &data;
    ictxt.page_data_db = &dbhandle;
    const char* xmlfile[] = 
    {
        "harvests1303818928.xml",
        "harvests1303819075.xml",
        "harvests1303819110.xml",
        "harvests1303819134.xml",
        "harvests1303819462.xml",
        "harvests1303819627.xml",
        "harvests1303819736.xml",
        ""
    };
    int         nr_exprs[] = 
    {
        1911,
        2281,
        2860,
        2149,
        1762,
        2201,
        1997,
        0,
    };
    string      xml_path;
    string dbenv_path = TMPDBENV_PATH;

    // Initializing Mws Xml Parser
    FAIL_ON(initxmlparser() != 0);
    // Initializing Mws Database
    FAIL_ON(dbhandle.init(dbenv_path) != 0);

    for(int i = 0; (string)xmlfile[i] != ""; i++)
    {
        xml_path = (string) MWS_TESTDATA_PATH +
                    "/" + (string) xmlfile[i];
        
        FAIL_ON((fd = open(xml_path.c_str(), O_RDONLY)) < 0);

        ret = loadMwsHarvestFromFd(fd, &ictxt).second;
        // Asserting if all the expressions have been parsed correctly
        FAIL_ON(ret != nr_exprs[i]);

        (void) close(fd);
    }


    (void) dbhandle.clean();
    (void) clearxmlparser();

    return EXIT_SUCCESS;

fail:
    return EXIT_FAILURE;
}

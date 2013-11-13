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
 * @file MemCrawlDb.cpp
 *
 */

#include "mws/dbc/MemCrawlDb.hpp"
#include "common/utils/macro_func.h"
#include <stdio.h>

using namespace std;
using namespace mws::dbc;

int main() {
    CrawlDb* crawlDb = new MemCrawlDb();
    const mws::CrawlData* crawlData;

    FAIL_ON(crawlDb->putData(3, "foobar") != 0);
    FAIL_ON(crawlDb->getData(4) != NULL);
    FAIL_ON((crawlData = crawlDb->getData(3)) == NULL);
    FAIL_ON(*crawlData != "foobar");

    delete crawlDb;

    return 0;

fail:
    return -1;
}

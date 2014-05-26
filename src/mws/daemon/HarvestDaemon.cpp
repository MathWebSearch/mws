/*Copyright (C) 2010-2013 KWARC Group <kwarc.info>

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
  * @brief File containing the implementation of the HarvestDaemon class.
  * @file HarvestDaemon.cpp
  * @author Corneliu-Claudiu Prodescu
  * @date 18 Jun 2011
  *
  * @edited Daniel Hasegan
  * @date 13 Aug 2012
  *
  * @edited Radu Hambasan
  * @date 13 Dec 2013
  *
  * License: GPL v3
  */

// System includes

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>  // Primitive System datatypes
#include <sys/stat.h>   // POSIX File characteristics
#include <fcntl.h>      // File control operations
#include <signal.h>
#include <stdlib.h>
#include <stack>

#include <string>
#include <vector>
// Local includes

#include "HarvestDaemon.hpp"
#include "common/socket/InSocket.hpp"
#include "common/socket/OutSocket.hpp"
#include "mws/dbc/LevFormulaDb.hpp"
#include "mws/dbc/LevCrawlDb.hpp"
#include "mws/dbc/MemFormulaDb.hpp"
#include "mws/dbc/MemCrawlDb.hpp"
#include "mws/index/TmpIndexAccessor.hpp"
#include "mws/dbc/DbQueryManager.hpp"
#include "mws/index/IndexBuilder.hpp"
#include "mws/index/ExpressionEncoder.hpp"
#include "mws/xmlparser/processMwsHarvest.hpp"
#include "mws/query/SearchContext.hpp"
#include "common/thread/ThreadWrapper.hpp"
#include "common/utils/Path.hpp"

using namespace std;
using namespace mws;
using namespace mws::types;
using namespace mws::index;

namespace mws {
namespace daemon {

MwsAnswset* HarvestDaemon::handleQuery(MwsQuery* mwsQuery) {
    MwsAnswset* result;
    query::SearchContext* ctxt;

    QueryEncoder encoder(meaningDictionary);
    vector<encoded_token_t> encodedQuery;
    ExpressionInfo queryInfo;

    if (encoder.encode(_config.indexingOptions, mwsQuery->tokens[0],
                       &encodedQuery, &queryInfo) == 0) {
        dbc::DbQueryManager dbQueryManger(crawlDb, formulaDb);
        ctxt = new query::SearchContext(encodedQuery);
        result = ctxt->getResult<TmpIndexAccessor>(
            data, &dbQueryManger, mwsQuery->attrResultLimitMin,
            mwsQuery->attrResultMaxSize, mwsQuery->attrResultTotalReqNr);
        delete ctxt;
    } else {
        result = new MwsAnswset();
    }

    result->qvarNames = queryInfo.qvarNames;
    result->qvarXpaths = queryInfo.qvarXpaths;

    return result;
}

int HarvestDaemon::initMws(const Config& config) {
    int ret = Daemon::initMws(config);

    if (config.useLevelDb) {
        auto crdb = new dbc::LevCrawlDb();
        auto fmdb = new dbc::LevFormulaDb();
        string crdbPath = config.dataPath + "/crawl.db";
        string fmdbPath = config.dataPath + "/formula.db";

        try {
            crdb->create_new(crdbPath.c_str(), config.deleteOldData);
            fmdb->create_new(fmdbPath.c_str(), config.deleteOldData);

            crawlDb = crdb;
            formulaDb = fmdb;
        } catch (const std::exception& e) {
            PRINT_WARN("Initializing database: %s\n", e.what());
            return EXIT_FAILURE;
        }
    } else {
        crawlDb = new dbc::MemCrawlDb();
        formulaDb = new dbc::MemFormulaDb();
    }

    data = new TmpIndex();
    meaningDictionary = new MeaningDictionary();

    indexBuilder = new index::IndexBuilder(
        formulaDb, crawlDb, data, meaningDictionary, config.indexingOptions);

    ret = ThreadWrapper::init();

    // load harvests
    const vector<string>& paths = config.harvestLoadPaths;
    vector<string>::const_iterator it;

    for (it = paths.begin(); it != paths.end(); it++) {
        AbsPath harvestPath(*it);
        printf("Loading from %s...\n", it->c_str());
        printf("%d expressions loaded.\n",
               parser::loadMwsHarvestFromDirectory(indexBuilder, harvestPath,
                                                   config.harvestFileExtension,
                                                   config.recursive));
        fflush(stdout);
    }

    return ret;
}

HarvestDaemon::HarvestDaemon()
    : indexBuilder(nullptr),
      meaningDictionary(nullptr),
      crawlDb(nullptr),
      formulaDb(nullptr),
      data(nullptr) {}

HarvestDaemon::~HarvestDaemon() {
    if (indexBuilder) delete indexBuilder;
    if (meaningDictionary) delete meaningDictionary;
    if (crawlDb) delete crawlDb;
    if (formulaDb) delete formulaDb;
    if (data) delete data;
}

}  // namespace daemon
}  // namespace mws

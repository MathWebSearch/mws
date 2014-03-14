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
  * @brief File containing the implementation of the IndexDaemon class.
  * @file IndexDaemon.cpp
  * @author Corneliu-Claudiu Prodescu
  * @date 18 Jun 2011
  *
  * @edited Radu Hambasan
  * @date 18 Feb 2014
  *
  * License: GPL v3
  */

// System includes

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>          // Primitive System datatypes
#include <sys/stat.h>           // POSIX File characteristics
#include <fcntl.h>              // File control operations
#include <signal.h>
#include <stdlib.h>

#include <mws/index/index.h>
#include <mws/query/engine.h>

#include <stack>
#include <fstream>
#include <vector>
#include <string>

// Local includes

#include "IndexDaemon.hpp"
#include "common/socket/InSocket.hpp"
#include "common/socket/OutSocket.hpp"
#include "mws/types/HandlerStruct.hpp"
#include "mws/dbc/LevFormulaDb.hpp"
#include "mws/dbc/LevCrawlDb.hpp"
#include "mws/xmlparser/loadMwsHarvestFromFd.hpp"
#include "mws/xmlparser/writeXmlAnswsetToFd.hpp"
#include "mws/xmlparser/initxmlparser.hpp"
#include "mws/xmlparser/clearxmlparser.hpp"
#include "mws/dbc/DbQueryManager.hpp"
#include "mws/index/ExpressionEncoder.hpp"

using namespace std;
using namespace mws;
using namespace mws::types;
using namespace mws::index;

namespace mws { namespace daemon {

static
result_cb_return_t result_callback(void* handle,
                                   const leaf_t * leaf) {
    if (leaf->type != LEAF_NODE) {
        fprintf(stderr, "Leaf callback error.");
        return QUERY_ERROR;
    }

    HandlerStruct *handlerStruct = (HandlerStruct *) handle;
    MwsAnswset *result = handlerStruct->result;
    MwsQuery   *mwsQuery = handlerStruct->mwsQuery;
    dbc::DbQueryManager* dbQueryManager = handlerStruct->dbQueryManager;

    dbc::DbAnswerCallback queryCallback =
            [result](const types::FormulaPath& formulaPath,
                     const types::CrawlData& crawlData) {
        mws::types::Answer* answer = new mws::types::Answer();
        answer->data = crawlData;
        answer->uri = formulaPath.xmlId;
        answer->xpath = formulaPath.xpath;
        result->answers.push_back(answer);
        return 0;
    };

    dbQueryManager->query((FormulaId)leaf->formula_id,
                          mwsQuery->attrResultLimitMin,
                          mwsQuery->attrResultMaxSize,
                          queryCallback);
    result->total += leaf->num_hits;

    return QUERY_CONTINUE;
}

MwsAnswset* IndexDaemon::handleQuery(MwsQuery* mwsQuery) {
    MwsAnswset* result = new MwsAnswset();
    dbc::DbQueryManager dbQueryManager(crawlDb, formulaDb);
    QueryEncoder encoder(meaningDictionary);
    std::vector<encoded_token_t> encFormula_vec;
    ExpressionInfo exprInfo;

    encoder.encode(mwsQuery->tokens[0], &encFormula_vec, &exprInfo);
    encoded_formula_t encFormula;
    encFormula.data = encFormula_vec.data();
    encFormula.size = encFormula_vec.size();

    HandlerStruct* handlerStruct = new HandlerStruct();
    handlerStruct->result = result;
    handlerStruct->mwsQuery = mwsQuery;
    handlerStruct->dbQueryManager = &dbQueryManager;

    query_engine_run(data, &encFormula, result_callback, handlerStruct);

    result->qvarNames = exprInfo.qvarNames;
    result->qvarXpaths = exprInfo.qvarXpaths;

    return result;
}


int IndexDaemon::initMws(const Config& config) {
    int ret = Daemon::initMws(config);
    dbc::LevCrawlDb* crdb = new dbc::LevCrawlDb();
    dbc::LevFormulaDb* fmdb = new dbc::LevFormulaDb();
    string crdbPath = config.dataPath + "/crawl.db";
    string fmdbPath = config.dataPath + "/formula.db";

    try {
        crdb->open(crdbPath.c_str());
        fmdb->open(fmdbPath.c_str());

        crawlDb = crdb;
        formulaDb = fmdb;
    }
    catch(const std::exception &e) {
        fprintf(stderr, "Initializing database: %s\n", e.what());
        return EXIT_FAILURE;
    }

    /*
     * Initializing data
     */
    string ms_path = config.dataPath + "/memsector.dat";
    memsector_handle_t msHandle;
    memsector_load(&msHandle, ms_path.c_str());

    data = new index_handle_t;
    *data = msHandle.index;

    /*
     * Initializing meaningDictionary
     */
    meaningDictionary = new MeaningDictionary();
    std::filebuf fb;
    std::istream os(&fb);
    fb.open((config.dataPath + "/meaning.dat").c_str(), std::ios::in);
    meaningDictionary->load(os);
    fb.close();

    return ret;
}

IndexDaemon::IndexDaemon() : data(NULL),
                             crawlDb(NULL),
                             formulaDb(NULL),
                             meaningDictionary(NULL) {
}

IndexDaemon::~IndexDaemon() {
    if (meaningDictionary) delete meaningDictionary;
    if (crawlDb) delete crawlDb;
    if (formulaDb) delete formulaDb;
    if (data) delete data;
}

}  // namespace daemon
}  // namespace mws

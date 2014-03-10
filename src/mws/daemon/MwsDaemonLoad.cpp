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
  * @brief File containing the implementation of the MwsDaemonLoad class.
  * @file MwsDaemonLoad.cpp
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

#include "MwsDaemonLoad.hpp"
#include "common/socket/InSocket.hpp"
#include "common/socket/OutSocket.hpp"
#include "mws/types/HandlerStruct.hpp"
#include "mws/dbc/LevFormulaDb.hpp"
#include "mws/dbc/LevCrawlDb.hpp"
#include "mws/xmlparser/loadMwsHarvestFromFd.hpp"
#include "mws/xmlparser/readMwsQueryFromFd.hpp"
#include "mws/xmlparser/writeXmlAnswsetToFd.hpp"
#include "mws/xmlparser/initxmlparser.hpp"
#include "mws/xmlparser/clearxmlparser.hpp"
#include "mws/xmlparser/writeJsonAnswsetToFd.hpp"
#include "common/types/ControlSequence.hpp"
#include "common/thread/ThreadWrapper.hpp"
#include "common/utils/DebugMacros.hpp"   // MWS Debug Macro Utilities
#include "common/utils/Path.hpp"
#include "common/utils/TimeStamp.hpp"     // MWS TimeStamp utility function
#include "mws/dbc/DbQueryManager.hpp"
#include "mws/index/ExpressionEncoder.hpp"

using namespace std;
using namespace mws;
using namespace mws::types;
using namespace mws::index;

static index_handle_t* data;
static InSocket* serverSocket;
static sig_atomic_t run = 1;
const string HarvestType = "mws:harvest";
const string QueryType = "mws:query";

dbc::CrawlDb* crawlDb;
dbc::FormulaDb* formulaDb;
MeaningDictionary* meaningDictionary;

namespace mws { namespace daemon {

static void
graceful_exit(int signum)
{
    switch (signum)
    {
        case SIGINT:
            exit(EXIT_SUCCESS);
        case SIGTERM:
            exit(EXIT_SUCCESS);
    }
}

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

static void*
HandleConnection(void* dataPtr)
{
    MwsQuery*         mwsQuery;
    MwsAnswset*       result;
    OutSocket*        outSocket;
    SocketInfo        sockInfo;
    int               ret;
    ControlSequence   controlSequence;
    int               fd;

    outSocket = (OutSocket*) dataPtr;
    sockInfo  = outSocket->getInfo();

    // Logging the connection
    printf("%19s "              "%35s"    "%25s\n",
            TimeStamp().c_str(),
            sockInfo.hostname.c_str(),
            sockInfo.service.c_str());
    fflush(stdout);

    // Reading the MwsQuery
    fd = outSocket->getFd();
    mwsQuery = readMwsQueryFromFd(fd);

    if (mwsQuery && mwsQuery->tokens.size()) {
#ifdef _APPLYRESTRICT
        mwsQuery->applyRestrictions();
#endif
        result = new MwsAnswset();

        dbc::DbQueryManager* dbQueryManager =
                new dbc::DbQueryManager(crawlDb, formulaDb);
        QueryEncoder encoder(meaningDictionary);
        std::vector<encoded_token_t> encFormula_vec;
        ExpressionInfo exprInfo;

        encoder.encode(mwsQuery->tokens[0], &encFormula_vec, &exprInfo);

        encoded_formula_t encFormula;
        encFormula.data = encFormula_vec.data();
        encFormula.size = encFormula_vec.size();

        result->qvarNames = exprInfo.qvarNames;
        result->qvarXpaths = exprInfo.qvarXpaths;

        HandlerStruct* handlerStruct = new HandlerStruct();
        handlerStruct->result = result;
        handlerStruct->mwsQuery = mwsQuery;
        handlerStruct->dbQueryManager = dbQueryManager;

        query_engine_run(data, &encFormula, result_callback, handlerStruct);


        // Sending the control sequence
        controlSequence.setFormat(mwsQuery->attrResultOutputFormat);
        controlSequence.send(outSocket->getFd());

        // Sending the answer with the proper format
        switch (mwsQuery->attrResultOutputFormat) {
            case DATAFORMAT_XML:
                ret = writeXmlAnswsetToFd(result,
                                          outSocket->getFd());
                break;
            case DATAFORMAT_JSON:
                ret = writeJsonAnswsetToFd(result,
                                           outSocket->getFd());
                break;
            default:
                ret = writeXmlAnswsetToFd(result,
                                          outSocket->getFd());
                break;
        }

        if (ret == -1) {
            fprintf(stderr, "Error while writing the Answer Set\n");
        }

        delete handlerStruct;
        delete result;
    } else {
        controlSequence.send(outSocket->getFd());
    }

    delete mwsQuery;
    delete outSocket;

    return NULL;
}


int initMws(const Config& config)
{
    int ret;
    if ((ret = initxmlparser())
            != 0)
    {
        fprintf(stderr, "Error while initializing xmlparser module\n");
        return 1;
    }

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

    ret = ThreadWrapper::init();
    if (ret)
    {
        fprintf(stderr, "Error while initializing thread module\n");
        clearxmlparser();
        return 1;
    }

    // Starting the network side and accepting connections
    serverSocket = new InSocket(config.mwsPort);
    // TODO check return
    serverSocket->enable();

    // Registering the signal handler
    signal(SIGTERM, graceful_exit);
    signal(SIGINT, graceful_exit);
    signal(SIGPIPE, SIG_IGN);

    return ret;
}


void cleanupMws()
{
    // Important to clean thread module first,
    // to wait for last connection threads to exit gracefully
    ThreadWrapper::clean();

    clearxmlparser();
    delete serverSocket;
    delete meaningDictionary;
    delete crawlDb;
    delete formulaDb;
    delete data;
}

int mwsDaemonLoopLoad(const Config& config)
{
    OutSocket* acceptedSock;

    if (initMws(config) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    atexit(cleanupMws);

    while (run)
    {
        acceptedSock = serverSocket->accept();
        ThreadWrapper::run(HandleConnection, acceptedSock);
    }

    return EXIT_SUCCESS;
}

}  // namespace daemon
}  // namespace mws

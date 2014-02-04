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
  * @brief File containing the implementation of the MwsDaemon class.
  * @file MwsDaemon.cpp
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
#include <sys/types.h>          // Primitive System datatypes
#include <sys/stat.h>           // POSIX File characteristics
#include <fcntl.h>              // File control operations
#include <signal.h>
#include <stdlib.h>
#include <stack>

// Local includes

#include "MwsDaemon.hpp"
#include "common/socket/InSocket.hpp"
#include "common/socket/OutSocket.hpp"
#include "mws/dbc/LevFormulaDb.hpp"
#include "mws/dbc/LevCrawlDb.hpp"
#include "mws/dbc/MemFormulaDb.hpp"
#include "mws/dbc/MemCrawlDb.hpp"
#include "mws/xmlparser/loadMwsHarvestFromFd.hpp"
#include "mws/xmlparser/readMwsQueryFromFd.hpp"
#include "mws/xmlparser/writeXmlAnswsetToFd.hpp"
#include "mws/xmlparser/initxmlparser.hpp"
#include "mws/xmlparser/clearxmlparser.hpp"
#include "mws/xmlparser/writeJsonAnswsetToFd.hpp"
#include "mws/index/MwsIndexNode.hpp"
#include "mws/query/SearchContext.hpp"
#include "common/types/ControlSequence.hpp"
#include "common/thread/ThreadWrapper.hpp"
#include "common/utils/DebugMacros.hpp"   // MWS Debug Macro Utilities
#include "common/utils/Path.hpp"
#include "common/utils/TimeStamp.hpp"     // MWS TimeStamp utility function
#include "mws/dbc/DbQueryManager.hpp"
#include "mws/index/IndexManager.hpp"

using namespace std;
using namespace mws;
using namespace mws::types;

static MwsIndexNode* data;
static InSocket* serverSocket;
static sig_atomic_t run = 1;
const string HarvestType = "mws:harvest";
const string QueryType = "mws:query";

dbc::CrawlDb* crawlDb;
dbc::FormulaDb* formulaDb;
MeaningDictionary* meaningDictionary;

index::IndexManager* indexManager;

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

static void*
HandleConnection(void* dataPtr)
{
    MwsQuery*         mwsQuery;
    MwsAnswset*       result;
    OutSocket*        outSocket;
    SocketInfo        sockInfo;
    SearchContext*    ctxt;
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

    if (mwsQuery && mwsQuery->tokens.size())
    {
#ifdef _APPLYRESTRICT
        mwsQuery->applyRestrictions();
#endif
        dbc::DbQueryManager dbQueryManger(crawlDb, formulaDb);
        ctxt   = new SearchContext(mwsQuery->tokens[0], meaningDictionary);

        result = ctxt->getResult(data,
                                 &dbQueryManger,
                                 mwsQuery->attrResultLimitMin,
                                 mwsQuery->attrResultMaxSize,
                                 mwsQuery->attrResultTotalReqNr);

        delete ctxt;

        // Sending the control sequence
        controlSequence.setFormat(mwsQuery->attrResultOutputFormat);
        controlSequence.send(outSocket->getFd());

        // Sending the answer with the proper format
        switch (mwsQuery->attrResultOutputFormat)
        {
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

        if (ret == -1)
        {
            fprintf(stderr, "Error while writing the Answer Set\n");
        }

        delete result;
    }
    else
    {
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

    if (config.useLevelDb) {
        dbc::LevCrawlDb* crdb = new dbc::LevCrawlDb();
        if (crdb->create_new("crawl1.db") == -1)
            crdb->open("crawl1.db");
        crawlDb = crdb;
    } else {
        crawlDb = new dbc::MemCrawlDb();
    }

    if (config.useLevelDb) {
        dbc::LevFormulaDb* fmdb = new dbc::LevFormulaDb();
        if (fmdb->create_new("formula1.db") == -1)
            fmdb->open("formula1.db");
        formulaDb = fmdb;
    } else {
        formulaDb = new dbc::MemFormulaDb();
    }

    data = new MwsIndexNode();
    meaningDictionary = new MeaningDictionary();

    indexManager = new index::IndexManager(formulaDb, crawlDb, data,
                                           meaningDictionary);

    ret = ThreadWrapper::init();
    if (ret)
    {
        fprintf(stderr, "Error while initializing thread module\n");
        clearxmlparser();
        return 1;
    }

    // load harvests
    const vector<string>& paths = config.harvestLoadPaths;
    vector<string> :: const_iterator it;
    for (it = paths.begin(); it != paths.end(); it++)
    {
        AbsPath harvestPath(*it);
        printf("Loading from %s...\n", it->c_str());
        printf("%d expressions loaded.\n",
                loadMwsHarvestFromDirectory(indexManager, harvestPath,
                                            config.recursive));
        fflush(stdout);
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
    delete data;
}


int mwsDaemonLoop(const Config& config)
{
    OutSocket* acceptedSock;

    initMws(config);

    atexit(cleanupMws);

    while (run)
    {
        acceptedSock = serverSocket->accept();
        ThreadWrapper::run(HandleConnection, acceptedSock);
    }
 
    return EXIT_SUCCESS;
}

} /* daemon */ } /* mws */

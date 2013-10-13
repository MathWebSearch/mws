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
  * License: GPL v3
  *
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
#include "mws/dbc/PageDbHandle.hpp"
#include "mws/xmlparser/verifyMwsMessageTypeFromFd.hpp"
#include "mws/xmlparser/loadMwsHarvestFromFd.hpp"
#include "mws/xmlparser/readMwsQueryFromFd.hpp"
#include "mws/xmlparser/verifyMwsMessageTypeFromMemory.hpp"
#include "mws/xmlparser/loadMwsHarvestFromMemory.hpp"
#include "mws/xmlparser/readMwsQueryFromMemory.hpp"
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

using namespace std;
using namespace mws;

static MwsIndexNode* data;
static PageDbHandle *dbhandle;
static InSocket* serverSocket;
static sig_atomic_t run = 1;
const string HarvestType = "mws:harvest";
const string QueryType = "mws:query";

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

static char*
readMessage(int fd);

static void*
HandleConnection(void* dataPtr)
{
    MwsQuery*         mwsQuery;
    MwsAnswset*       result;
    OutSocket*        outSocket;
    SocketInfo        sockInfo;
    stack<CmmlToken*> exprStack;
    SearchContext*    ctxt;
    int               ret;
    ControlSequence   controlSequence;
    char *            message;
    string            messageType;
    int               fd;
    pair<int,int>     loadReturn;
    
    outSocket = (OutSocket*) dataPtr;
    sockInfo  = outSocket->getInfo();

    // Logging the connection
    printf("%19s "              "%35s"    "%25s\n",
            TimeStamp().c_str(),
            sockInfo.hostname.c_str(),
            sockInfo.service.c_str());
    fflush(stdout);
 
    fd = outSocket->getFd();
    message = readMessage(fd);

    //Print MessageType
    messageType = verifyMwsMessageTypeFromMemory(message);
    printf("Type: %s\n",messageType.c_str());
    fflush(stdout);

    // IN CASE OF MWS HARVEST
    if (messageType == HarvestType)
    {
        printf("Processing Harvest ....");

        loadReturn = loadMwsHarvestFromMemory(data, message, dbhandle);
        if (loadReturn.first == 0)
        {
            printf("%d loaded\n", loadReturn.second);
        }
        else
        {
            printf("%d loaded (with errors)\n", loadReturn.second);
        }
        fflush(stdout);


       // totalLoaded += loadReturn.second;
      free(message);
      delete outSocket;
      return NULL;
    }
    else if (messageType != QueryType)
    {
      free(message);
      delete outSocket;
      return NULL;
    }

    // IN CASE OF MWS QUERY

    // Reading the MwsQuery
    mwsQuery = readMwsQueryFromMemory(message);
    free(message);

    if (mwsQuery && mwsQuery->tokens.size())
    {
#ifdef _APPLYRESTRICT
        mwsQuery->applyRestrictions();
#endif

        ctxt   = new SearchContext(mwsQuery->tokens[0]);

        result = ctxt->getResult(data,
                                 dbhandle,
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

static char*
readMessage(int fd)
{
    FILE*             fdstream;
    size_t            lSize = 1000000;
    size_t            bytes_read;
    char *            message;


    // Duplicate file descriptor
    int fd2 = dup(fd);

    // Open file stream reader on fd2
    if (  NULL == (fdstream = fdopen( fd2 ,"r" ))  ) {
        fprintf(stderr,"Cannot open file descriptor %d\n", fd);
        return NULL;
    }

    // allocate memory to contain the first lSize of the file:
    message = (char*) malloc (sizeof(char)*lSize);
    if (message == NULL) {
        fprintf (stderr,"Memory allocation fail.\n");
        return NULL;
    }
    bytes_read = fread(message, 1, lSize, fdstream);


    while (!feof(fdstream)) {
        bytes_read += fread(message + bytes_read, 1, lSize - bytes_read, fdstream);
        if (bytes_read == lSize) {
            lSize = 2 * lSize;
            char* new_message = (char*) malloc (sizeof(char)*lSize);
            memcpy(new_message, message, bytes_read);
            free(message);
            message = new_message;
        }
    }

    fclose(fdstream);

    message[bytes_read] = '\0';

    return message;
}


int initMws(const Config& config)
{
    int ret;

    if ((ret = MeaningDictionary::init())
            != 0)
    {
        fprintf(stderr, "Error while initializing MeaningDictionary\n");
        return 1;
    }

    data = new MwsIndexNode();

    if ((ret = initxmlparser())
            != 0)
    {
        fprintf(stderr, "Error while initializing xmlparser module\n");
        MeaningDictionary::clean();
        return 1;
    }

    dbhandle = new PageDbHandle();

    ret = dbhandle->init(config.dataPath);
    if (ret)
    {
        fprintf(stderr, "Error while initializing dbc module\n");
        clearxmlparser();
        MeaningDictionary::clean();
        return 1;
    }

    ret = ThreadWrapper::init();
    if (ret)
    {
        fprintf(stderr, "Error while initializing thread module\n");
        dbhandle->clean();
        clearxmlparser();
        MeaningDictionary::clean();
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
                loadMwsHarvestFromDirectory(data, harvestPath, dbhandle,
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
    dbhandle->clean();
    delete serverSocket;
    delete dbhandle;
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

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
  * @brief File containing the implementation of the Daemon class.
  * @file Daemon.cpp
  * @author Radu Hambasan
  * @date 10 Mar 2014
  *
  * License: GPL v3
  */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>          // Primitive System datatypes
#include <sys/stat.h>           // POSIX File characteristics
#include <fcntl.h>              // File control operations
#include <signal.h>
#include <stdlib.h>

#include <stack>
#include <string>

#include "common/socket/InSocket.hpp"
#include "common/socket/OutSocket.hpp"
#include "common/thread/ThreadWrapper.hpp"
#include "common/utils/DebugMacros.hpp"   // MWS Debug Macro Utilities
#include "common/utils/Path.hpp"
#include "common/utils/TimeStamp.hpp"     // MWS TimeStamp utility function
#include "mws/xmlparser/clearxmlparser.hpp"
#include "mws/xmlparser/initxmlparser.hpp"
#include "common/types/ControlSequence.hpp"
#include "mws/query/SearchContext.hpp"
#include "mws/xmlparser/writeJsonAnswsetToFd.hpp"
#include "mws/xmlparser/writeXmlAnswsetToFd.hpp"
#include "mws/xmlparser/loadMwsHarvestFromFd.hpp"
#include "mws/xmlparser/readMwsQueryFromFd.hpp"

#include "Daemon.hpp"

namespace mws { namespace daemon {

typedef struct handler_s {
    Daemon* daemon;
    OutSocket* acceptedSock;
} handler_t;

Daemon::Daemon() : run(1), HarvestType("mws:harvest"), QueryType("mws:query") {
}

static void
graceful_exit(int signum) {
    switch (signum) {
        case SIGINT:
            exit(EXIT_SUCCESS);
        case SIGTERM:
            exit(EXIT_SUCCESS);
    }
}

static void cleanupMws() {
    // Important to clean thread module first,
    // to wait for last connection threads to exit gracefully
    ThreadWrapper::clean();
    clearxmlparser();
}

static void* HandleConnection(void* dataPtr) {
    handler_t* handl = (handler_t*) dataPtr;

    MwsQuery*         mwsQuery;
    MwsAnswset*       result;
    OutSocket*        outSocket;
    SocketInfo        sockInfo;
    int               ret;
    ControlSequence   controlSequence;
    int               fd;

    Daemon* instance = handl->daemon;
    outSocket = handl->acceptedSock;
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
        // get the result
        result = instance->handleQuery(mwsQuery);
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

        delete result;
    } else {
        controlSequence.send(outSocket->getFd());
    }

    delete handl;
    delete mwsQuery;
    delete outSocket;

    return NULL;
}
Daemon::~Daemon() {
    delete serverSocket;
}

int Daemon::loop(const Config& config) {
    if (initMws(config) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    atexit(cleanupMws);

    while (run) {
        handler_t* handl = new handler_t;
        handl->daemon = this;
        handl->acceptedSock = serverSocket->accept();
        ThreadWrapper::run(HandleConnection, handl);
    }

    return EXIT_SUCCESS;
}

int Daemon::initMws(const Config &config) {
    int ret;
    if ((ret = initxmlparser())!= 0) {
        fprintf(stderr, "Error while initializing xmlparser module\n");
        return 1;
    }


    ret = ThreadWrapper::init();
    if (ret) {
        fprintf(stderr, "Error while initializing thread module\n");
        clearxmlparser();
        return 1;
    }


    // Starting the network side and accepting connections
    serverSocket = new InSocket(config.mwsPort);
    serverSocket->enable();

    // Registering the signal handler
    signal(SIGTERM, graceful_exit);
    signal(SIGINT, graceful_exit);
    signal(SIGPIPE, SIG_IGN);

    return ret;
}

}  // namespace daemon
}  // namespace mws

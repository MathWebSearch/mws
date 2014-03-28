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
/*
 * Main crawlerd executable that starts the Crawler Daemon
 */

// System includes
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <signal.h>                    // C signals headers
#include <unistd.h>                    // C Misc functions
#include <stdint.h>                    // C standard integer types

// Local includes
#include "crawler/daemon/CrawlerDaemon.hpp"
#include "crawler/types/SharedQueue.hpp" 
#include "crawler/crawler/MwsCrawler.hpp"
#include "common/utils/FlagParser.hpp"
#include "common/thread/ThreadWrapper.hpp"

#include "build-gen/config.h"

using namespace std;
using namespace common::utils;

// struct needed for the Crawler
struct CrawlerInfo{
    SharedQueue* sharedQueuePtr;
    string dataDirectory;
};

// sigQuit is the Flag that tells us to terminate or not
static volatile sig_atomic_t sigQuit = 0;
static void catch_sigint(int sig)
{
    if (sig == SIGINT || sig == SIGTERM)
        sigQuit = 1;
}

static void* 
startCrawler(void* dataPtr)
{
    CrawlerInfo* ci = (CrawlerInfo*) dataPtr;
    SharedQueue *sq = (SharedQueue*) ci->sharedQueuePtr;

    mws::Crawler::run(sq,ci->dataDirectory.c_str());

    return NULL;
}

int main(int argc, char* argv[])
{
    CrawlerDaemon         crawlerDaemon;
    sigset_t              mask,old_mask;
    struct sigaction      sa,old_sa1,old_sa2;
    int                   restPort;
    int                   ret;
    SharedQueue           *sq;
    string                dataDir;

    // FOR DEBUG:
    sq = new SharedQueue;

    // Parsing the flags
    FlagParser::addFlag('p', "port",    FLAG_REQ, ARG_REQ );
    FlagParser::addFlag('d', "data-directory", FLAG_OPT, ARG_REQ);

    if ((ret = FlagParser::parse(argc, argv))
            != 0)
    {
        fprintf(stderr, "%s", FlagParser::getUsage().c_str());
        exit(0);
    }

    restPort = atoi(FlagParser::getArg('p').c_str());
    if (restPort <= 0 || restPort > 65535)
    {
        fprintf(stderr,
                "\"%s\" is not a valid port number\n",
                FlagParser::getArg('p').c_str());
        exit(0);
    }

    // get the Data Directory for the crawler
    dataDir = string(HARVESTFILES_PATH);
    if (FlagParser::hasArg('d'))
    {
        dataDir = dataDir + "/" + (FlagParser::getArg('d'));
    }

    // Initialize Thread
    ret = ThreadWrapper::init();
    if (ret)
    {
        perror("");
        fprintf(stderr,"Failed to initialize Thread!\n");
        exit(0);
    }

    // Start the Crawler on a second Thread
    CrawlerInfo *ci = new CrawlerInfo;
    ci->sharedQueuePtr = sq;
    ci->dataDirectory = dataDir;
    ThreadWrapper::run( *startCrawler , (void*) ci);

    // Starting the daemon
    ret = crawlerDaemon.start(restPort,sq);
    if (ret != 0)
    {
        fprintf(stderr, "Failure while starting the Crawler Daemon\n");
        exit(0);
    }

    // Preparing the Signal Action
    sa.sa_handler = catch_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    // Preparing the signal mask
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    // Block the signals and actions
    if (sigprocmask(SIG_BLOCK,&mask,&old_mask) == -1)
        fprintf(stderr,"sigprocmask - SIG_BLOCK");
    if (sigaction(SIGINT, &sa, &old_sa1) == -1)
        fprintf(stderr,"sigaction - open");
    if (sigaction(SIGTERM, &sa, &old_sa2) == -1)
        fprintf(stderr,"sigaction - open");

    // Waiting for SIGINT / SIGTERM
    while (!sigQuit)
        sigsuspend(&old_mask);

    // UNBLOCK the signals and actions
    if (sigprocmask(SIG_SETMASK,&old_mask,NULL) == -1)
        fprintf(stderr,"sigprocmask - SIG_SETMASK");
    if (sigaction(SIGINT, &old_sa1, NULL) == -1)
        fprintf(stderr,"sigaction - close");
    if (sigaction(SIGINT, &old_sa2, NULL) == -1)
        fprintf(stderr,"sigaction - close");

    crawlerDaemon.stop();


    // delete sharedQueue
    while ( !(*sq).sharedQueue.empty() )
    {
        sq->sharedQueue.pop();
    }

    return EXIT_SUCCESS;
}

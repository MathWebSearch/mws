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
#ifndef _MWSCRAWLERDAEMON_H
#define _MWSCRAWLERDAEMON_H

/**
  * @brief File containing the header of the CrawlerDaemon class.
  * @file CrawlerDaemon.h
  * @author Daniel Hasegan
  * @date 20 Aug 2012
  *
  * License: GPL v3
  *
  */

// System includes

#include <microhttpd.h>
#include <vector>
#include <string>

// Local includes
#include "common/thread/ThreadWrapper.hpp"
#include "crawler/types/SharedQueue.hpp"

#define POSTBUFFERSIZE  512
#define MAXNAMESIZE     100
#define MAXANSWERSIZE   512
#define MAXASKPAGESIZE  4096
#define MAXDONTCRAWLSIZE 128

#define GET             0
#define POST            1

/*
 * Connection info struct
 */
struct connection_info_struct
{
    int connectiontype;
    char *urlstart;
    char *count;
    int dontcrawlnr;
    char *dontcrawl[MAXDONTCRAWLSIZE];
    struct MHD_PostProcessor *postprocessor;
};

/**
  * @brief Class encapsulating a basic RESTful interface daemon.
  */
class CrawlerDaemon
{
private:
    struct MHD_Daemon* _daemonHandler;
    SharedQueue *sharedQueue;
public:
    /**
      * @brief Default Constructor of the CrawlerDaemon class.
      */
    CrawlerDaemon();

    /**
      * @brief Default Destructor of the CrawlerDaemon class.
      */
    ~CrawlerDaemon();

    /**
      * @brief Method to start a crawler daemon.
      * @param aPort the port where to run.
      * @return 0 on success and -1 on failure.
      */
    int start(int aPort,SharedQueue* sqPtr);

    /**
      * @brief Method to stop a running daemon.
      */
    void stop();
};

#endif

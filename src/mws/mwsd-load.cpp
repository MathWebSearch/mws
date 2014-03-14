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
  * @brief MathWebSearch daemon load executable
  * @file mwsd-load.cpp
  * @author Corneliu-Claudiu Prodescu
  * @date 18 Jun 2011
  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "common/utils/FlagParser.hpp"
#include "common/utils/save_pid_file.h"
#include "mws/daemon/IndexDaemon.hpp"
#include "mws/index/memsector.h"
#include "config.h"

using std::vector;
using std::string;
using common::utils::FlagParser;

int main(int argc, char* argv[]) {
    int ret;
    mws::daemon::Config config;
    mws::daemon::IndexDaemon daemon;

    // Parsing the flags
    FlagParser::addFlag('m', "mws-port",             FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('I', "index-path",           FLAG_REQ, ARG_REQ);
    FlagParser::addFlag('i', "pid-file",             FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('l', "log-file",             FLAG_OPT, ARG_REQ);
#ifndef __APPLE__
    FlagParser::addFlag('d', "daemonize",            FLAG_OPT, ARG_NONE);
#endif  // !__APPLE__

    if ((ret = FlagParser::parse(argc, argv)) != 0) {
        fprintf(stderr, "%s", FlagParser::getUsage().c_str());
        goto failure;
    }

    // mws-port
    if (FlagParser::hasArg('m')) {
        int mwsPort = atoi(FlagParser::getArg('m').c_str());
        if (mwsPort > 0 && mwsPort < (1<<16)) {
            config.mwsPort = mwsPort;
        } else {
            fprintf(stderr, "Invalid port \"%s\"\n",
                    FlagParser::getArg('m').c_str());
            goto failure;
        }
    } else {
        fprintf(stderr, "Using default mws port %d\n", DEFAULT_MWS_PORT);
        config.mwsPort = DEFAULT_MWS_PORT;
    }

    // index-path
        config.dataPath = FlagParser::getArg('I').c_str();

    // log-file
    if (FlagParser::hasArg('l')) {
        fprintf(stderr, "Redirecting output to %s\n",
                FlagParser::getArg('l').c_str());
        if (freopen(FlagParser::getArg('l').c_str(), "w", stderr) == NULL) {
            fprintf(stderr, "ERROR: Unable to redirect stderr to %s\n",
                    FlagParser::getArg('l').c_str());
            goto failure;
        }
        if (freopen(FlagParser::getArg('l').c_str(), "w", stdout) == NULL) {
            fprintf(stderr, "ERROR: Unable to redirect stdout to %s\n",
                    FlagParser::getArg('l').c_str());
            goto failure;
        }
    }

#ifndef __APPLE__
    // daemon
    if (FlagParser::hasArg('d')) {
        // Daemonizing
        ret = ::daemon(0, /* noclose = */ FlagParser::hasArg('l'));
        if (ret != 0) {
            fprintf(stderr, "Error while daemonizing\n");
            goto failure;
        }
    }
#endif  // !__APPLE__

    // pid-file - always needs to be done after daemonizing
    if (FlagParser::hasArg('i')) {
        ret = save_pid_file(FlagParser::getArg('i').c_str());
        if (ret != 0) {
            fprintf(stderr, "ERROR: Unable to save pidfile %s\n",
                    FlagParser::getArg('i').c_str());
            goto failure;
        }
    }

    return daemon.loop(config);

failure:
    return EXIT_FAILURE;
}

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
  * @brief File containing the main function of Restful interface of the
  * MathWebSearch Project
  * @file restd.cpp
  * @author Corneliu-Claudiu Prodescu
  * @date 20 Jun 2011
  * @updated Daniel Hasegan
  *
  * License: GPL v3
  *
  */

// System includes

#include <signal.h>                    // C signals headers
#include <stdlib.h>                    // C general purpose headers
#include <stdio.h>                     // C standard input output headers
#include <unistd.h>                    // C Misc functions

// Local includes

#include "common/utils/FlagParser.hpp" // Flag parser utility class
#include "common/utils/save_pid_file.h"
#include "rest/daemon/RestDaemon.hpp"  // Rest Daemon headers

// Config

#include "MwsDaemonConf.hpp"

// Variables 
static volatile sig_atomic_t sigQuit = 0;

static void catch_sigint(int sig)
{
  if (sig == SIGINT || sig == SIGTERM)
    sigQuit = 1;
}


int main(int argc, char* argv[])
{
    RestDaemon            restDaemon;
    sigset_t              mask,old_mask;
    struct sigaction      sa,old_sa1,old_sa2;
    int                   port;
    int                   ret;
    RestDaemon::Config    config;

    // Parsing the flags
    FlagParser::addFlag('p', "rest-port",   FLAG_REQ, ARG_REQ );
    FlagParser::addFlag('m', "mws-port",    FLAG_OPT, ARG_REQ );
    FlagParser::addFlag('h', "mws-host",    FLAG_OPT, ARG_REQ );
    FlagParser::addFlag('d', "daemonize",   FLAG_OPT, ARG_NONE);
    FlagParser::addFlag('i', "pid-file",    FLAG_OPT, ARG_REQ );
    FlagParser::addFlag('l', "log-file",    FLAG_OPT, ARG_REQ );

    if ((ret = FlagParser::parse(argc, argv))
            != 0)
    {
        fprintf(stderr, "%s", FlagParser::getUsage().c_str());
        goto failure;
    }
 
    // rest port
    port = atoi(FlagParser::getArg('p').c_str());
    if (port <= 0 || port > 65535) {
        fprintf(stderr,
                "\"%s\" is not a valid port number\n",
                FlagParser::getArg('p').c_str());
        goto failure;
    }
    config.restPort = port;

    // mws host
    if (FlagParser::hasArg('h')) {
        config.mwsHost = FlagParser::getArg('h');
    } else {
        config.mwsHost = DEFAULT_MWS_HOST;
    }

    // mws port
    if (FlagParser::hasArg('m')) {
        port = atoi(FlagParser::getArg('m').c_str());
        if (port <= 0 || port > 65535) {
            fprintf(stderr,
                    "\"%s\" is not a valid port number\n",
                    FlagParser::getArg('m').c_str());
            goto failure;
        }
        config.mwsPort = port;
    } else {
        config.mwsPort = DEFAULT_MWS_PORT;
    }

    // daemon
    if (FlagParser::hasArg('d'))
    {
        // Daemonizing
        ret = daemon(0, /* noclose = */ FlagParser::hasArg('l'));
        if (ret != 0)
        {
            fprintf(stderr, "Error while daemonizing\n");
            goto failure;
        }
    }

    // pid-file - always needs to be done after daemonizing
    if (FlagParser::hasArg('i')) {
        ret = save_pid_file(FlagParser::getArg('i').c_str());
        if (ret != 0) {
            fprintf(stderr, "ERROR: Unable to save pidfile %s\n",
                    FlagParser::getArg('i').c_str());
            goto failure;
        }
    }

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

    // Starting the daemon
    ret = restDaemon.startAsync(config);
    if (ret != 0) {
        fprintf(stderr, "Failure while starting the Rest Daemon\n");
        goto failure;
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

    restDaemon.stop();
    return EXIT_SUCCESS;

failure:
    return EXIT_FAILURE;
}

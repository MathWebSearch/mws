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
  * @brief MathWebSearch daemon executable
  * @file mwsd.cpp
  * @author Corneliu-Claudiu Prodescu
  * @date 18 Jun 2011
  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <string>
using std::string;
#include <vector>
using std::vector;
#include <memory>
using std::unique_ptr;

#include "common/utils/save_pid_file.h"
#include "common/utils/FlagParser.hpp"
using common::utils::FlagParser;
#include "daemon/Daemon.hpp"
using mws::daemon::Daemon;
using mws::daemon::Config;
#include "mws/daemon/HarvestDaemon.hpp"
using mws::daemon::HarvestDaemon;
#include "mws/daemon/IndexDaemon.hpp"
using mws::daemon::IndexDaemon;
#include "index/IndexBuilder.hpp"
using mws::index::IndexingConfiguration;
#include "mws/index/memsector.h"
#include "mws/index/IndexWriter.hpp"
using mws::index::createCompressedIndex;

#include "build-gen/config.h"

static volatile sig_atomic_t shouldQuit = 0;
unique_ptr<Daemon> mwsDaemon;

static void catch_sig(int sig);
static void setup_signals();
static void wait_for_signal();

int main(int argc, char* argv[]) {
    int ret;
    Config config;

    setup_signals();

    // Parsing the flags
    FlagParser::addFlag('I', "include-harvest-path", FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('x', "experimental-query-engine", FLAG_OPT, ARG_NONE);
    FlagParser::addFlag('p', "mws-port", FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('D', "data-path", FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('i', "pid-file", FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('l', "log-file", FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('r', "recursive", FLAG_OPT, ARG_NONE);
    FlagParser::addFlag('c', "enable-ci-renaming", FLAG_OPT, ARG_NONE);
    FlagParser::addFlag('e', "harvest-file-extension", FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('f', "delete-old-data", FLAG_OPT, ARG_NONE);
    FlagParser::addFlag('6', "enable-ipv6", FLAG_OPT, ARG_NONE);
#ifndef __APPLE__
    FlagParser::addFlag('d', "daemonize", FLAG_OPT, ARG_NONE);
#endif  // !__APPLE__

    if ((ret = FlagParser::parse(argc, argv)) != 0) {
        fprintf(stderr, "%s", FlagParser::getUsage().c_str());
        goto failure;
    }

    // either -I or -D must be given
    if (!(FlagParser::hasArg('D')) && !(FlagParser::hasArg('I'))) {
        fprintf(stderr, "%s", FlagParser::getUsage().c_str());
        goto failure;
    }

    // harvest paths
    if (FlagParser::hasArg('I')) {
        config.harvestLoadPaths = FlagParser::getArgs('I');
        config.indexingConfiguration.harvestLoadPaths = config.harvestLoadPaths;
    }
    config.enableIpv6 = FlagParser::hasArg('6');

    // harvest file extension
    if (FlagParser::hasArg('e')) {
        config.harvestFileExtension = FlagParser::getArg('e');
    } else {
        config.harvestFileExtension = DEFAULT_MWS_HARVEST_SUFFIX;
    }
    config.indexingConfiguration.harvestFileExtension =
        config.harvestFileExtension;

    // recursive
    config.recursive = FlagParser::hasArg('r');
    config.indexingConfiguration.recursive = config.recursive;
    // leveldb
    config.useLevelDb = FlagParser::hasArg('L');

    // ci renaming
    config.indexingOptions.renameCi = FlagParser::hasArg('c');
    config.indexingConfiguration.indexingOptions = config.indexingOptions;

    // mws-port
    if (FlagParser::hasArg('p')) {
        int mwsPort = atoi(FlagParser::getArg('p').c_str());
        if (mwsPort > 0 && mwsPort < (1 << 16)) {
            config.mwsPort = mwsPort;
        } else {
            fprintf(stderr, "Invalid port \"%s\"\n",
                    FlagParser::getArg('p').c_str());
            goto failure;
        }
    } else {
        fprintf(stderr, "Using default mws port %d\n", DEFAULT_MWS_PORT);
        config.mwsPort = DEFAULT_MWS_PORT;
    }

    // log-file
    if (FlagParser::hasArg('l')) {
        fprintf(stderr, "Redirecting output to %s\n",
                FlagParser::getArg('l').c_str());
        if (freopen(FlagParser::getArg('l').c_str(), "w", stderr) == nullptr) {
            fprintf(stderr, "ERROR: Unable to redirect stderr to %s\n",
                    FlagParser::getArg('l').c_str());
            goto failure;
        }
        if (freopen(FlagParser::getArg('l').c_str(), "w", stdout) == nullptr) {
            fprintf(stderr, "ERROR: Unable to redirect stdout to %s\n",
                    FlagParser::getArg('l').c_str());
            goto failure;
        }
    }

    // should delete old data
    config.indexingConfiguration.deleteOldIndex = FlagParser::hasArg('f');

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

    // data-path
    if (FlagParser::hasArg('D')) {
        config.dataPath = FlagParser::getArg('D');
        config.indexingConfiguration.dataPath = config.dataPath;

        mwsDaemon.reset(new IndexDaemon());
        ret = mwsDaemon->startAsync(config);
        if (ret != 0) {
            if (!(FlagParser::hasArg('I'))) {
                PRINT_WARN("Index not found and no harvest directory provided. "
                           "Aborting...\n");
                goto failure;
            }
            PRINT_LOG("Index not found. Attempting to build it...\n");

            if (createCompressedIndex(config.indexingConfiguration) != 0) {
                PRINT_WARN("Could not build index. Aborting...\n");
                goto failure;
            }

            // retry with the built index
            if (mwsDaemon->startAsync(config) != 0) {
                PRINT_WARN(
                    "An error occured while loading the fresh index...\n");
                goto failure;
            }
        }
        PRINT_LOG("Index loaded successfully. \n");
    } else {
        PRINT_LOG("Using default data path %s\n", DEFAULT_MWS_DATA_PATH);
        PRINT_LOG("Trying to load index in memory.\n");
        config.dataPath = DEFAULT_MWS_DATA_PATH;
        mwsDaemon.reset(new HarvestDaemon());
        ret = mwsDaemon->startAsync(config);
        if (ret != 0) {
            PRINT_WARN("Failure while starting the daemon\n");
            goto failure;
        }
    }

    wait_for_signal();
    mwsDaemon->stop();

    return EXIT_SUCCESS;

failure:
    return EXIT_FAILURE;
}

static void catch_sig(int sig) {
    switch (sig) {
    case SIGINT:
    case SIGTERM:
        shouldQuit = 1;
        break;
    case SIGSEGV:
    case SIGABRT:
        mwsDaemon->stop();
        // unmap the index
        delete mwsDaemon.release();
        PRINT_LOG("Index was unmmapped.\n");
        // disable handling for SIGTERM AND SIGABRT
        struct sigaction new_sa;
        memset(&new_sa, 0, sizeof(struct sigaction));
        new_sa.sa_handler = SIG_DFL;
        sigaction(SIGTERM, &new_sa, nullptr);
        sigaction(SIGABRT, &new_sa, nullptr);
        // let it crash and generate a core dump
        raise(sig);
        break;
    }
}

static void setup_signals() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));

    sa.sa_handler = catch_sig;
    sigaction(SIGINT,  &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGABRT, &sa, nullptr);
}

static void wait_for_signal() {
    sigset_t mask, old_mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    sigprocmask(SIG_BLOCK, &mask, &old_mask);
    // Waiting for SIGINT / SIGTERM
    while (!shouldQuit) sigsuspend(&old_mask);

    sigprocmask(SIG_SETMASK, &old_mask, nullptr);
}

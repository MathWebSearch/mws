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
#include <stdexcept>
using std::exception;

#include "common/utils/save_pid_file.h"
#include "common/utils/FlagParser.hpp"
using common::utils::FlagParser;
#include "mws/daemon/Daemon.hpp"
using mws::daemon::Daemon;
#include "mws/daemon/HarvestQueryHandler.hpp"
using mws::daemon::HarvestQueryHandler;
#include "mws/daemon/IndexQueryHandler.hpp"
using mws::daemon::IndexQueryHandler;
using mws::daemon::QueryHandler;
#include "mws/index/IndexBuilder.hpp"
using mws::index::HarvesterConfiguration;
#include "mws/index/IndexWriter.hpp"
using mws::index::IndexConfiguration;
using mws::index::createCompressedIndex;
#include "mws/index/memsector.h"
#include "mws/xmlparser/xmlparser.hpp"
using mws::parser::initxmlparser;

#include "build-gen/config.h"

static volatile sig_atomic_t shouldQuit = 0;
unique_ptr<Daemon> mwsDaemon;

static void catch_sig(int sig);
static void setup_signals();
static void wait_for_signal();

int main(int argc, char* argv[]) {
    int ret;
    Daemon::Config daemonConfig;
    IndexConfiguration indexConfig;

    // Parsing the flags
    FlagParser::addFlag('I', "include-harvest-path", FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('n', "ignore-harvest-data", FLAG_OPT, ARG_NONE);
    FlagParser::addFlag('D', "data-path", FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('x', "experimental-query-engine", FLAG_OPT, ARG_NONE);
    FlagParser::addFlag('p', "mws-port", FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('i', "pid-file", FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('l', "log-file", FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('r', "recursive", FLAG_OPT, ARG_NONE);
    FlagParser::addFlag('c', "enable-ci-renaming", FLAG_OPT, ARG_NONE);
    FlagParser::addFlag('e', "harvest-file-extension", FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('f', "delete-old-data", FLAG_OPT, ARG_NONE);
    FlagParser::addFlag('6', "enable-ipv6", FLAG_OPT, ARG_NONE);
    FlagParser::addFlag('s', "log-index-stats", FLAG_OPT, ARG_REQ);
#ifndef __APPLE__
    FlagParser::addFlag('d', "daemonize", FLAG_OPT, ARG_NONE);
#endif  // !__APPLE__

    if ((ret = FlagParser::parse(argc, argv)) != 0) {
        fprintf(stderr, "%s", FlagParser::getUsage().c_str());
        return EXIT_FAILURE;
    }

    // ipv6
    daemonConfig.enableIpv6 = FlagParser::hasArg('6');

    // port
    if (FlagParser::hasArg('p')) {
        int mwsPort = atoi(FlagParser::getArg('p').c_str());
        if (mwsPort > 0 && mwsPort < (1 << 16)) {
            daemonConfig.port = mwsPort;
        } else {
            fprintf(stderr, "Invalid port \"%s\"\n",
                    FlagParser::getArg('p').c_str());
            return EXIT_FAILURE;
        }
    }

    // harvest paths
    if (FlagParser::hasArg('I')) {
        indexConfig.harvester.paths = FlagParser::getArgs('I');
    }

    // harvest file extension
    if (FlagParser::hasArg('e')) {
        indexConfig.harvester.fileExtension = FlagParser::getArg('e');
    }

    // harvest recursively
    indexConfig.harvester.recursive = FlagParser::hasArg('r');

    // ignore harvest data
    indexConfig.harvester.shouldIgnoreData = FlagParser::hasArg('n');

    // ci renaming
    indexConfig.harvester.encoding.renameCi = FlagParser::hasArg('c');

    // log-file
    if (FlagParser::hasArg('l')) {
        fprintf(stderr, "Redirecting output to %s\n",
                FlagParser::getArg('l').c_str());
        if (freopen(FlagParser::getArg('l').c_str(), "w", stdout) == nullptr) {
            fprintf(stderr, "ERROR: Unable to redirect stdout to %s\n",
                    FlagParser::getArg('l').c_str());
            return EXIT_FAILURE;
        }
        if (dup2(STDOUT_FILENO, STDERR_FILENO) < 0) {
            fprintf(stderr, "ERROR: Unable to redirect stderr to %s\n",
                    FlagParser::getArg('l').c_str());
            return EXIT_FAILURE;
        }
    }

    // should delete old data
    indexConfig.deleteOldData = FlagParser::hasArg('f');

    if (FlagParser::hasArg('s')) {
        indexConfig.harvester.statisticsLogFile = FlagParser::getArg('s');
    }

#ifndef __APPLE__
    // daemon
    if (FlagParser::hasArg('d')) {
        // Daemonizing
        ret = ::daemon(0, /* noclose = */ FlagParser::hasArg('l'));
        if (ret != 0) {
            fprintf(stderr, "Failed to daemonize\n");
            return EXIT_FAILURE;
        }
    }
#endif  // !__APPLE__

    // pid-file - always needs to be done after daemonizing
    if (FlagParser::hasArg('i')) {
        ret = save_pid_file(FlagParser::getArg('i').c_str());
        if (ret != 0) {
            fprintf(stderr, "ERROR: Unable to save pidfile %s\n",
                    FlagParser::getArg('i').c_str());
            return EXIT_FAILURE;
        }
    }

    // data-path and harvest paths
    if (FlagParser::hasArg('D')) {
        try {
            indexConfig.dataPath = FlagParser::getArg('D');
            IndexQueryHandler::Config config;
            config.encoding = indexConfig.harvester.encoding;
            config.useExperimentalQueryEngine = FlagParser::hasArg('x');
            QueryHandler* qh = nullptr;

            try {
                qh = new IndexQueryHandler(indexConfig.dataPath, config);
            } catch (const exception& e) {
                PRINT_LOG("Could not load index: %s\n", e.what());
            }

            if (qh == nullptr && FlagParser::hasArg('I')) {
                PRINT_LOG("Attempting to build it from harvests\n");
                createCompressedIndex(indexConfig);
                qh = new IndexQueryHandler(indexConfig.dataPath, config);
            } else if (!FlagParser::hasArg('I')) {
                PRINT_WARN("Invalid index and no harvests supplied."\
                           "Aborting...\n");
                return EXIT_FAILURE;
            }

            PRINT_LOG("Index loaded successfully.\n");
            setup_signals();
            mwsDaemon.reset(new Daemon(qh, daemonConfig));
        } catch (const exception& e) {
            PRINT_WARN("Aborting: %s", e.what());
            return EXIT_FAILURE;
        }
    } else if (FlagParser::hasArg('I')) {
        try {
            initxmlparser();
            HarvestQueryHandler* queryHandler;
            queryHandler = new HarvestQueryHandler(indexConfig.harvester);
            PRINT_LOG("Index built successfully.\n");
            setup_signals();
            mwsDaemon.reset(new Daemon(queryHandler, daemonConfig));
        } catch (const exception& e) {
            PRINT_WARN("Aborting: %s", e.what());
            return EXIT_FAILURE;
        }
    } else {
        fprintf(stderr, "At least one of the flags -I and -D is required\n%s",
                FlagParser::getUsage().c_str());
        return EXIT_FAILURE;
    }

    wait_for_signal();
    mwsDaemon.reset();

    return EXIT_SUCCESS;
}

static void catch_sig(int sig) {
    switch (sig) {
    case SIGINT:
    case SIGTERM:
        shouldQuit = 1;
        break;
    case SIGSEGV:
    case SIGABRT:
        // delete mwsDaemon
        mwsDaemon.reset();
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
    sigaction(SIGINT, &sa, nullptr);
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

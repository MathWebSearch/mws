/*

Copyright (C) 2010-2014 KWARC Group <kwarc.info>

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
  * @brief SchemaSearch daemon executable
  * @file schemad.cpp
  * @author Radu Hambasan
  * @date 30 Dec 2014
  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>


#include <memory>
using std::unique_ptr;
#include <stdexcept>
using std::exception;
#include <string>
using std::string;
#include <vector>
using std::vector;

#include "common/utils/save_pid_file.h"
#include "common/utils/FlagParser.hpp"
using common::utils::FlagParser;
#include "mws/daemon/Daemon.hpp"
using mws::daemon::Daemon;
#include "mws/index/ExpressionEncoder.hpp"
#include "mws/daemon/SchemaQueryHandler.hpp"
using mws::daemon::SchemaQueryHandler;
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
    mws::index::ExpressionEncoder::Config encodingConfig;

    // Parsing the flags
    FlagParser::addFlag('p', "schema-port", FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('i', "pid-file", FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('l', "log-file", FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('c', "enable-ci-renaming", FLAG_OPT, ARG_NONE);
    FlagParser::addFlag('6', "enable-ipv6", FLAG_OPT, ARG_NONE);
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
    daemonConfig.port = DEFAULT_SCHEMA_PORT;
    if (FlagParser::hasArg('p')) {
        int schPort= atoi(FlagParser::getArg('p').c_str());
        if (schPort> 0 && schPort< (1 << 16)) {
            daemonConfig.port = schPort;
        } else {
            fprintf(stderr, "Invalid port \"%s\"\n",
                    FlagParser::getArg('p').c_str());
            return EXIT_FAILURE;
        }
    }

    // ci renaming
    encodingConfig.renameCi = FlagParser::hasArg('c');

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

    try {
        initxmlparser();
        SchemaQueryHandler* queryHandler =
                new SchemaQueryHandler(encodingConfig);
        setup_signals();
        mwsDaemon.reset(new Daemon(queryHandler, daemonConfig));
    } catch (const exception& e) {
        PRINT_WARN("Aborting: %s", e.what());
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
        PRINT_LOG("Schemad exited successfully.");
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

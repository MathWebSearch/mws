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
#ifndef _MWS_DAEMON_DAEMON_HPP
#define _MWS_DAEMON_DAEMON_HPP

/**
  * @brief File containing the header of the Daemon class.
  * @file Daemon.hpp
  * @author Radu Hambasan
  * @date 10 Mar 2014
  *
  * License: GPL v3
  */

#include <signal.h>
#include "mws/daemon/microhttpd_linux.h"

#include <vector>
#include <string>

#include "mws/types/MwsAnswset.hpp"
#include "mws/types/Query.hpp"
#include "mws/index/IndexBuilder.hpp"
#include "mws/index/IndexWriter.hpp"

namespace mws { namespace daemon {

struct Config {
    uint16_t mwsPort;
    bool enableIpv6;
    index::IndexConfiguration index;
    bool useExperimentalQueryEngine;

    Config();
};

class Daemon {
 public:
    int startAsync(const Config& config);
    void stop();
    virtual MwsAnswset* handleQuery(types::Query* query) = 0;
    Daemon();
    virtual ~Daemon();

 protected:
    virtual int initMws(const Config& config);
    Config _config;
 private:
    struct MHD_Daemon* _daemonHandler;
};
}  // namespace daemon
}  // namespace mws

#endif

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

#include <vector>
#include <string>

#include "common/socket/InSocket.hpp"
#include "common/socket/OutSocket.hpp"
#include "mws/types/MwsAnswset.hpp"
#include "mws/types/MwsQuery.hpp"

namespace mws { namespace daemon {

struct Config {
    std::vector<std::string> harvestLoadPaths;
    bool                     recursive;
    uint16_t                 mwsPort;
    std::string              dataPath;
    bool                     useLevelDb;
    std::string              harvestFileExtension;
};

class Daemon {
 public:
    int loop(const Config& config);
    virtual MwsAnswset* handleQuery(MwsQuery* query) = 0;
    Daemon();
    virtual ~Daemon();

 protected:
    virtual int initMws(const Config& config);

 protected:
    InSocket* serverSocket;
    sig_atomic_t run;
    std::string HarvestType;
    const std::string QueryType;
};
}  // namespace daemon
}  // namespace mws

#endif

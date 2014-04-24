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
#ifndef _INDEX_DAEMON_HPP
#define _INDEX_DAEMON_HPP

/**
  * @brief File containing the header of the IndexDaemon class.
  * @file IndexDaemon.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date 18 Jun 2011
  *
  * @edited Radu Hambasan
  * @date 18 Feb 2014
  *
  * License: GPL v3
  *
  */

#include "mws/index/index.h"

#include "Daemon.hpp"
#include "mws/dbc/FormulaDb.hpp"
#include "mws/dbc/CrawlDb.hpp"
#include "mws/dbc/LevFormulaDb.hpp"
#include "mws/dbc/LevCrawlDb.hpp"
#include "mws/index/MeaningDictionary.hpp"
#include "mws/index/IndexManager.hpp"

namespace mws { namespace daemon {

class IndexDaemon : public Daemon {
 public:
    IndexDaemon();
    ~IndexDaemon();
 private:
    MwsAnswset* handleQuery(MwsQuery *query);
    int initMws(const Config& config);
 private:
    index_handle_t* data;
    dbc::CrawlDb* crawlDb;
    dbc::FormulaDb* formulaDb;
    index::MeaningDictionary* meaningDictionary;
};
}  // namespace daemon
}  // namespace mws

#endif  // _INDEX_DAEMON_HPP

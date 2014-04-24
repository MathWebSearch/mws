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
#ifndef _HARVEST_DAEMON_HPP
#define _HARVEST_DAEMON_HPP

/**
  * @brief File containing the header of the HarvestDaemon class.
  * @file HarvestDaemon.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date 18 Jun 2011
  *
  * License: GPL v3
  *
  */

#include "Daemon.hpp"
#include "mws/dbc/FormulaDb.hpp"
#include "mws/dbc/CrawlDb.hpp"
#include "mws/dbc/LevFormulaDb.hpp"
#include "mws/dbc/LevCrawlDb.hpp"
#include "mws/index/MwsIndexNode.hpp"
#include "mws/index/MeaningDictionary.hpp"
#include "mws/index/IndexManager.hpp"

namespace mws { namespace daemon {

class HarvestDaemon: public Daemon {
 public :
    HarvestDaemon();
    ~HarvestDaemon();
 private:
    MwsAnswset* handleQuery(MwsQuery* query);
    int initMws(const Config& config);
 private:
    index::IndexManager* indexManager;
    index::MeaningDictionary* meaningDictionary;
    dbc::CrawlDb* crawlDb;
    dbc::FormulaDb* formulaDb;
    MwsIndexNode* data;
};
}}

#endif  // _HARVEST_DAEMON_HPP

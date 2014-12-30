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
#ifndef _MWS_DAEMON_HARVESTQUERYHANDLER_HPP
#define _MWS_DAEMON_HARVESTQUERYHANDLER_HPP

/**
  * @brief HarvestQueryHandler interface
  * @file HarvestQueryHandler.hpp
  * @author Corneliu-Claudiu Prodescu
  * @date 15 Jun 2014
  */

#include "mws/daemon/QueryHandler.hpp"
#include "mws/dbc/MemFormulaDb.hpp"
#include "mws/dbc/MemCrawlDb.hpp"
#include "mws/index/TmpIndex.hpp"
#include "mws/index/MeaningDictionary.hpp"
#include "mws/index/IndexBuilder.hpp"

namespace mws {
namespace daemon {

class HarvestQueryHandler: public QueryHandler {
 public:
    explicit HarvestQueryHandler(const index::HarvesterConfiguration& config);
    ~HarvestQueryHandler();

    GenericAnswer* handleQuery(types::Query* query);

 private:
    index::MeaningDictionary _meaningDictionary;
    dbc::MemCrawlDb _crawlDb;
    dbc::MemFormulaDb _formulaDb;
    index::TmpIndex _index;
    index::ExpressionEncoder::Config _encodingConfig;

    DISALLOW_COPY_AND_ASSIGN(HarvestQueryHandler);
};

}  // namespace daemon
}  // namespace mws

#endif  // _MWS_DAEMON_HARVESTQUERYHANDLER_HPP

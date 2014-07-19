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
#ifndef _MWS_INDEX_INDEXLOADER_HPP
#define _MWS_INDEX_INDEXLOADER_HPP

/**
  * @file IndexLoader.hpp
  * @brief Indexing Manager
  * @date 18 Nov 2013
  */

#include <string>
#include <memory>

#include "mws/types/CmmlToken.hpp"
#include "mws/dbc/FormulaDb.hpp"
#include "mws/dbc/CrawlDb.hpp"
#include "mws/dbc/DbQueryManager.hpp"
#include "mws/index/MeaningDictionary.hpp"
#include "mws/index/index.h"

namespace mws {
namespace index {

struct LoadingOptions {
    bool includeHits;
    LoadingOptions() : includeHits(true) {}
};

class IndexLoader {
 public:
    /**
     * @brief Method to load an index stored on disk
     */
    IndexLoader(const std::string& indexPath,
                const LoadingOptions& options = LoadingOptions());

    ~IndexLoader();

    dbc::FormulaDb* getFormulaDb();
    dbc::DbQueryManager* getDbQueryManager();
    index_handle_t* getIndexHandle();
    index::MeaningDictionary* getMeaningDictionary();

 private:
    index::MeaningDictionary m_meaningDictionary;
    std::unique_ptr<dbc::FormulaDb> m_formulaDb;
    std::unique_ptr<dbc::CrawlDb> m_crawlDb;
    std::unique_ptr<dbc::DbQueryManager> m_dbQueryManager;
    index_handle_t m_index;
    memsector_handle_t m_memsectorHandler;

    DISALLOW_COPY_AND_ASSIGN(IndexLoader);
};

}  // namespace index
}  // namespace mws

#endif  // _MWS_INDEX_INDEXLOADER_HPP

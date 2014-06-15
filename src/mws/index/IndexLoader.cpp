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
  * @file IndexLoader.cpp
  * @brief Indexing Manager implementation
  * @date 18 Nov 2013
  */

#include <assert.h>
#include <string.h>

#include <set>
using std::set;
#include <stack>
using std::stack;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <unordered_map>
using std::unordered_map;
#include <memory>
using std::unique_ptr;
#include <stdexcept>
using std::runtime_error;

#include "mws/dbc/LevCrawlDb.hpp"
using mws::dbc::CrawlDb;
using mws::dbc::LevCrawlDb;
#include "mws/dbc/LevFormulaDb.hpp"
using mws::dbc::FormulaDb;
using mws::dbc::LevFormulaDb;
#include "mws/dbc/DbQueryManager.hpp"
using mws::dbc::DbQueryManager;
#include "mws/types/CmmlToken.hpp"
using mws::types::CmmlToken;
using mws::types::TokenCallback;
#include "mws/types/FormulaPath.hpp"
using mws::types::FormulaId;
using mws::types::FormulaPath;
#include "mws/dbc/CrawlDb.hpp"
using mws::dbc::CrawlId;
using mws::dbc::CrawlData;
#include "mws/index/ExpressionEncoder.hpp"
#include "mws/index/IndexLoader.hpp"

#include "build-gen/config.h"

namespace mws {
namespace index {

IndexLoader::IndexLoader(const std::string& path, const LoadingOptions& options)
    : m_meaningDictionary(path + "/" + MEANING_DICTIONARY_FILE) {

    // we need the two databases two include hits
    if (options.includeHits) {
        auto formulaDb = new LevFormulaDb();
        m_formulaDb = unique_ptr<FormulaDb>(formulaDb);
        formulaDb->open((path + "/" + FORMULA_DB_FILE).c_str());
        PRINT_LOG("Loaded FormulaDb\n");

        auto crawlDb = new LevCrawlDb();
        m_crawlDb = unique_ptr<CrawlDb>(crawlDb);
        crawlDb->open((path + "/" + CRAWL_DB_FILE).c_str());
        PRINT_LOG("Loaded CrawlDb\n");

        m_dbQueryManager =
                unique_ptr<DbQueryManager>(new DbQueryManager(crawlDb,
                                                              formulaDb));
    }

    if (memsector_load(&m_memsectorHandler,
                       (path + "/" + INDEX_MEMSECTOR_FILE).c_str()) != 0) {
        throw runtime_error("Error while loading memsector " + path + "/" +
                            INDEX_MEMSECTOR_FILE);
    }
    PRINT_LOG("Loaded Index\n");

    m_index.ms = m_memsectorHandler.ms;
    m_index.root = memsector_get_root(&m_memsectorHandler);
}

IndexLoader::~IndexLoader() { memsector_unload(&m_memsectorHandler); }

dbc::DbQueryManager* IndexLoader::getDbQueryManager() {
    return m_dbQueryManager.get();
}

index_handle_t* IndexLoader::getIndexHandle() { return &m_index; }

MeaningDictionary* IndexLoader::getMeaningDictionary() {
    return &m_meaningDictionary;
}

FormulaDb* IndexLoader::getFormulaDb() { return m_formulaDb.get(); }

}  // namespace index
}  // namespace mws

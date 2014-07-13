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
  * @file IndexBuilder.cpp
  * @brief Indexing Builder implementation
  * @date 18 Nov 2013
  */

#include <assert.h>
#include <fcntl.h>

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
#include "mws/index/IndexBuilder.hpp"
#include "mws/index/IndexIterator.hpp"
#include "mws/index/TmpIndexAccessor.hpp"
#include "mws/xmlparser/processMwsHarvest.hpp"
using mws::parser::loadMwsHarvestFromFd;
#include "common/utils/util.hpp"
using common::utils::foreachEntryInDirectory;

namespace mws {
namespace index {

static void logIndexStatistics(const TmpIndex* index, FILE* logFile);

IndexBuilder::IndexBuilder(dbc::FormulaDb* formulaDb, dbc::CrawlDb* crawlDb,
                           TmpIndex* index,
                           MeaningDictionary* meaningDictionary,
                           EncodingConfiguration encodingOptions)
    : m_formulaDb(formulaDb),
      m_crawlDb(crawlDb),
      m_index(index),
      m_meaningDictionary(meaningDictionary),
      m_indexingOptions(std::move(encodingOptions)) {}

CrawlId IndexBuilder::indexCrawlData(const CrawlData& crawlData) {
    return m_crawlDb->putData(crawlData);
}

int IndexBuilder::indexContentMath(const CmmlToken* cmmlToken,
                                   const string xmlId, const CrawlId& crawlId) {
    assert(cmmlToken != nullptr);
    set<FormulaId> uniqueFormulaIds;
    HarvestEncoder encoder(m_meaningDictionary);
    int numSubExpressions = 0;

    cmmlToken->foreachSubexpression([&](const CmmlToken* token) {
        vector<encoded_token_t> encodedFormula;
        encoder.encode(m_indexingOptions, token, &encodedFormula, nullptr);
        TmpLeafNode* leaf = m_index->insertData(encodedFormula);
        FormulaId formulaId = leaf->id;
        auto ret = uniqueFormulaIds.insert(formulaId);
        if (ret.second) {
            types::FormulaPath formulaPath;
            formulaPath.xmlId = xmlId;
            formulaPath.xpath = token->getXpath();
            m_formulaDb->insertFormula(leaf->id, crawlId, formulaPath);
            leaf->solutions++;
            numSubExpressions++;
        }
    });

    return numSubExpressions;
}

uint64_t loadHarvests(mws::index::IndexBuilder* indexBuilder,
                      const index::HarvesterConfiguration& config) {
    uint64_t numExpressions = 0;
    FILE* logFile = nullptr;

    if (config.statisticsLogFile != "") {
        logFile = fopen(config.statisticsLogFile.c_str(), "w");
        if (logFile == nullptr) {
            perror(config.statisticsLogFile.c_str());
        } else {
            PRINT_LOG("Dumping index statistics to %s\n",
                      config.statisticsLogFile.c_str());
            fprintf(logFile, "#%12s %12s %12s\n", "<formulae>", "<unique>",
                    "<size>");
        }
    }

    for (string dirPath : config.paths) {
        PRINT_LOG("Loading from %s...\n", dirPath.c_str());
        common::utils::FileCallback fileCallback = [&](
            const std::string& path, const std::string& prefix) {
            UNUSED(prefix);
            if (common::utils::hasSuffix(path, config.fileExtension)) {
                PRINT_LOG("Loading %s... ", path.c_str());
                int fd = open(path.c_str(), O_RDONLY);
                if (fd < 0) {
                    perror(path.c_str());
                    return -1;
                }
                auto loadReturn = loadMwsHarvestFromFd(indexBuilder, fd);
                if (loadReturn.first == 0) {
                    PRINT_LOG("%d loaded\n", loadReturn.second);
                } else {
                    PRINT_LOG("%d loaded (with errors)\n", loadReturn.second);
                }
                numExpressions += loadReturn.second;
                close(fd);
            } else {
                PRINT_LOG("Skipping \"%s\": bad extension\n", path.c_str());
            }
            if (logFile != nullptr) {
                logIndexStatistics(indexBuilder->getIndex(), logFile);
            }

            return 0;
        };

        common::utils::DirectoryCallback recursive = [&](
            const std::string partialPath) {
            UNUSED(partialPath);
            return config.recursive;
        };

        if (foreachEntryInDirectory(dirPath, fileCallback, recursive) != 0) {
            continue;
        }
    }

    if (logFile != nullptr) {
        fclose(logFile);
    }

    return numExpressions;
}

static void logIndexStatistics(const TmpIndex* index, FILE* logFile) {
    assert(logFile != nullptr);

    uint64_t expressions = 0;
    uint64_t uniqueExpressions = 0;
    uint64_t indexSize = index->computeMemsectorSize();

    IndexIterator<TmpIndexAccessor> iterator(index);
    while (auto leaf = static_cast<const TmpLeafNode*>(iterator.next())) {
        expressions += leaf->getNumSolutions();
        uniqueExpressions++;
    }
    fprintf(logFile, " %12" PRIu64 " %12" PRIu64 " %12" PRIu64 "\n",
            expressions, uniqueExpressions, indexSize);
}

}  // namespace index
}  // namespace mws

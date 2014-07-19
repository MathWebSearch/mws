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

#include <cinttypes>
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
using mws::parser::HarvestProcessor;
using mws::parser::HarvestResult;
#include "common/utils/util.hpp"
using common::utils::foreachEntryInDirectory;

#include "build-gen/config.h"

namespace mws {
namespace index {

static void logIndexStatistics(const TmpIndex* index, FILE* logFile);

HarvesterConfiguration::HarvesterConfiguration()
    : recursive(false), fileExtension(DEFAULT_MWS_HARVEST_SUFFIX) {}

IndexBuilder::IndexBuilder(dbc::FormulaDb* formulaDb, dbc::CrawlDb* crawlDb,
                           TmpIndex* index,
                           MeaningDictionary* meaningDictionary,
                           ExpressionEncoder::Config encodingOptions)
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

uint64_t loadHarvests(IndexBuilder* indexBuilder,
                      const HarvesterConfiguration& config) {
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
                auto loadReturn = loadHarvestFromFd(indexBuilder, fd);
                PRINT_LOG("%" PRIu64 " loaded", loadReturn.numExpressions);
                if (loadReturn.status == 0) {
                    PRINT_LOG("\n");
                } else {
                    PRINT_LOG(" (with errors)\n");
                }
                numExpressions += loadReturn.numExpressions;
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

HarvestResult loadHarvestFromFd(IndexBuilder* indexBuilder, int fd) {
    class HarvestIndexer : public HarvestProcessor {
     public:
        explicit HarvestIndexer(IndexBuilder* indexBuilder)
                : _IndexBuilder(indexBuilder) {}
        int processExpression(const CmmlToken* token, const string& exprUri,
                              const uint32_t& crawlId) {
            return _IndexBuilder->indexContentMath(token, exprUri, crawlId);
        }
        CrawlId processData(const string& data) {
            return _IndexBuilder->indexCrawlData(data);
        }

     private:
        IndexBuilder* _IndexBuilder;
    };

    HarvestIndexer harvestIndexer(indexBuilder);

    return processHarvestFromFd(fd, &harvestIndexer);
}

}  // namespace index
}  // namespace mws

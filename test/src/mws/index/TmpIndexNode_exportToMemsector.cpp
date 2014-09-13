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
 * @file TmpIndexNode_exportToMemsector.cpp
 *
 */

#include <errno.h>
#include <unistd.h>

#include <cinttypes>
#include <string>
using std::string;

#include "mws/dbc/MemCrawlDb.hpp"
#include "mws/dbc/MemFormulaDb.hpp"
#include "mws/index/TmpIndex.hpp"
using mws::index::TmpIndexNode;
using mws::index::TmpLeafNode;
using mws::index::TmpIndex;
#include "mws/index/index.h"
#include "mws/index/IndexBuilder.hpp"
using mws::index::loadHarvests;
#include "mws/index/IndexWriter.hpp"
using mws::index::HarvesterConfiguration;
#include "mws/index/MeaningDictionary.hpp"
using mws::index::MeaningDictionary;
#include "common/utils/compiler_defs.h"
#include "common/utils/FlagParser.hpp"
using common::utils::FlagParser;

#include "build-gen/config.h"

#define DEFAULT_TMP_MEMSECTOR_PATH "/tmp/test.memsector"
#define DEFAULT_HARVEST_PATH MWS_TESTDATA_PATH
/* force use of both types of internal nodes */
#undef MEMSECTOR_LONG_OFF_START
#define MEMSECTOR_LONG_OFF_START (1UL << 8)

using namespace mws;

struct Tester {
    static const memsector_header_t* ms;

    static inline bool memsector_inode_consistent(
        const TmpIndexNode* tmp_node, memsector_long_off_t baseOff) {
        if (tmp_node->children.size() > 0) {  // child
            inode_t* inode = (inode_t*)memsector_off2addr(ms, baseOff);

            if (inode->type != INTERNAL_NODE &&
                inode->type != LONG_INTERNAL_NODE) {
                PRINT_LOG("inode at offset %" PRIu64 " corrupted!\n", baseOff);
                return false;
            }
            if (tmp_node->children.size() != inode->size) return false;

            int i = 0;
            for (auto& kv : tmp_node->children) {
                MeaningId meaningId = kv.first.id;
                Arity arity = kv.first.arity;

                encoded_token_t token;
                if (inode->type == LONG_INTERNAL_NODE) {
                    token = ((inode_long_t*)inode)->data[i].token;
                } else {
                    token = inode->data[i].token;
                }
                if (meaningId != token.id) return false;
                if (arity != token.arity) return false;

                i++;
            }

            i = 0;
            for (auto& kv : tmp_node->children) {
                const TmpIndexNode* child_node = kv.second;
                memsector_long_off_t offset;
                if (inode->type == LONG_INTERNAL_NODE) {
                    offset = ((inode_long_t*)inode)->data[i].off;
                } else {
                    offset = inode->data[i].off;
                }
                if (!memsector_inode_consistent(child_node, baseOff - offset)) {
                    return false;
                }

                i++;
            }
            return true;
        } else {  // leaf
            leaf_t* leaf = (leaf_t*)memsector_off2addr(ms, baseOff);
            if (leaf->type != LEAF_NODE) {
                PRINT_LOG("leaf node at offset %" PRIu64 " corrupted!\n",
                          baseOff);
                return false;
            }
            TmpLeafNode* tmpLeaf = (TmpLeafNode*)tmp_node;
            return ((tmpLeaf->id == leaf->formula_id) &&
                    (tmpLeaf->solutions == leaf->num_hits));
        }
    }

    static inline int test_memsector_consistency(TmpIndex* data,
                                                 memsector_handle_t* msHandle) {
        ms = msHandle->ms;
        if (Tester::memsector_inode_consistent(data->mRoot, ms->root_off))
            return 0;
        else
            return -1;
    }
};

const memsector_header_t* Tester::ms;

int main(int argc, char* argv[]) {
    memsector_writer_t mswr;
    memsector_handle_t ms;
    dbc::CrawlDb* crawlDb;
    dbc::FormulaDb* formulaDb;
    TmpIndex data;
    MeaningDictionary* meaningDictionary;
    index::IndexBuilder* indexBuilder;
    index::ExpressionEncoder::Config indexEncoding;
    string tmp_memsector_path;
    HarvesterConfiguration config;

    FlagParser::addFlag('I', "include-harvest-path", FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('O', "tmp-memsector-path", FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('c', "enable-ci-renaming", FLAG_OPT, ARG_REQ);

    if (FlagParser::parse(argc, argv) != 0) {
        PRINT_LOG("%s", FlagParser::getUsage().c_str());
        goto fail;
    }

    if (FlagParser::hasArg('I')) {
        config.paths.push_back(FlagParser::getArg('I'));
    } else {
        PRINT_LOG("Using default include harvest path %s\n",
                  DEFAULT_HARVEST_PATH);
        config.paths.push_back(DEFAULT_HARVEST_PATH);
    }
    config.fileExtension = "harvest";
    config.recursive = false;

    if (FlagParser::hasArg('O')) {
        tmp_memsector_path = FlagParser::getArg('O');
    } else {
        PRINT_LOG("Using default temporary memsector path %s\n",
                  DEFAULT_TMP_MEMSECTOR_PATH);
        tmp_memsector_path = DEFAULT_TMP_MEMSECTOR_PATH;
    }

    if (FlagParser::hasArg('c')) {
        indexEncoding.renameCi = true;
    } else {
        indexEncoding.renameCi = false;
    }

    crawlDb = new dbc::MemCrawlDb();
    formulaDb = new dbc::MemFormulaDb();
    meaningDictionary = new MeaningDictionary();
    indexBuilder = new index::IndexBuilder(formulaDb, crawlDb, &data,
                                           meaningDictionary, indexEncoding);

    /* ensure the file does not exist */
    FAIL_ON(unlink(tmp_memsector_path.c_str()) != 0 && errno != ENOENT);

    FAIL_ON(loadHarvests(indexBuilder, config) <= 0);
    FAIL_ON(memsector_create(&mswr, tmp_memsector_path.c_str()) != 0);
    data.exportToMemsector(&mswr);
    printf("Index exported to memsector %s (%" PRIu64 "b)\n",
           tmp_memsector_path.c_str(), mswr.ms.index_size);

    FAIL_ON(memsector_load(&ms, tmp_memsector_path.c_str()) != 0);
    printf("Memsector loaded\n");

    if (Tester::test_memsector_consistency(&data, &ms) != 0) {
        printf("FAIL: Inconsistency detected!\n");
        goto fail;
    }
    printf("Memsector consistent with index\n");

    FAIL_ON(memsector_remove(&ms) != 0);
    printf("Memsector removed\n");

    return 0;

fail:
    return -1;
}

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
#include "mws/index/memsector.h"
#include "mws/index/IndexManager.hpp"
#include "mws/index/MeaningDictionary.hpp"
using mws::index::MeaningDictionary;
#include "mws/xmlparser/processMwsHarvest.hpp"
#include "common/utils/compiler_defs.h"
#include "common/utils/FlagParser.hpp"
using common::utils::FlagParser;

#include "build-gen/config.h"

#define DEFAULT_TMP_MEMSECTOR_PATH  "/tmp/test.memsector"
#define DEFAULT_HARVEST_PATH        MWS_TESTDATA_PATH

using namespace mws;

const memsector_alloc_header_t *alloc;

struct Tester {
    static inline
    bool memsector_inode_consistent(const TmpIndexNode* tmp_node,
                                    const inode_t* inode) {
        switch (inode->type) {
        case INTERNAL_NODE: {
            if (tmp_node->children.size() != inode->size) return false;
            int i = 0;
            for (auto& kv : tmp_node->children) {
                MeaningId           meaningId  = kv.first.id;
                Arity               arity      = kv.first.arity;
                const TmpIndexNode* child_node = kv.second;

                if (meaningId != inode->data[i].token.id) return false;
                if (arity     != inode->data[i].token.arity) return false;
                inode_t* child_inode = (inode_t*)
                        memsector_off2addr(alloc, inode->data[i].off);
                if (!memsector_inode_consistent(child_node, child_inode)) {
                    return false;
                }

                i++;
            }
            return true;
        }

        case LEAF_NODE: {
            leaf_t *leaf = (leaf_t*) inode;
            TmpLeafNode* tmpLeaf = (TmpLeafNode*) tmp_node;
            return ((tmpLeaf->id == leaf->formula_id) &&
                    (tmpLeaf->solutions == leaf->num_hits));
        }

        default: {
            assert(false);
        }
        }
    }

    static inline
    int test_memsector_consistency(TmpIndex* data, memsector_handle_t* ms) {
        alloc = ms_get_alloc(ms);
        if (Tester::memsector_inode_consistent(data->mRoot, ms->index.root))
            return 0;
        else
            return -1;
    }
};

int main(int argc, char* argv[]) {
    memsector_writer_t mswr;
    memsector_handle_t ms;
    dbc::CrawlDb* crawlDb;
    dbc::FormulaDb* formulaDb;
    TmpIndex data;
    MeaningDictionary* meaningDictionary;
    index::IndexManager* indexManager;
    index::IndexingOptions indexingOptions;
    string harvest_path;
    string tmp_memsector_path;
    uint64_t memsector_size;

    FlagParser::addFlag('I', "include-harvest-path",    FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('O', "tmp-memsector-path",      FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('c', "enable-ci-renaming",      FLAG_OPT, ARG_REQ);

    if (FlagParser::parse(argc, argv) != 0) {
        PRINT_LOG("%s", FlagParser::getUsage().c_str());
        goto fail;
    }

    if (FlagParser::hasArg('I')) {
        harvest_path = FlagParser::getArg('I');
    } else {
        PRINT_LOG("Using default include harvest path %s\n",
                DEFAULT_HARVEST_PATH);
        harvest_path = DEFAULT_HARVEST_PATH;
    }

    if (FlagParser::hasArg('O')) {
        tmp_memsector_path = FlagParser::getArg('O');
    } else {
        PRINT_LOG("Using default temporary memsector path %s\n",
                DEFAULT_TMP_MEMSECTOR_PATH);
        tmp_memsector_path = DEFAULT_TMP_MEMSECTOR_PATH;
    }

    if (FlagParser::hasArg('c')) {
        indexingOptions.renameCi = true;
    } else {
        PRINT_LOG("Not renaming ci\n");
        indexingOptions.renameCi = false;
    }

    crawlDb = new dbc::MemCrawlDb();
    formulaDb = new dbc::MemFormulaDb();
    meaningDictionary = new MeaningDictionary();
    indexManager = new index::IndexManager(formulaDb, crawlDb,
                                           &data, meaningDictionary,
                                           indexingOptions);

    /* ensure the file does not exist */
    FAIL_ON(unlink(tmp_memsector_path.c_str()) != 0 && errno != ENOENT);

    FAIL_ON(parser::loadMwsHarvestFromDirectory(indexManager,
                                                AbsPath(harvest_path),
                                                ".harvest",
                                                /* recursive = */ false) <= 0);

    memsector_size = data.getMemsectorSize();
    FAIL_ON(memsector_create(&mswr, tmp_memsector_path.c_str(),
                             memsector_size) != 0);
    printf("Memsector %s of %" PRIu64 " Kb created\n",
           tmp_memsector_path.c_str(), memsector_size / 1024);
    
    data.exportToMemsector(&mswr);
    printf("Index exported to memsector\n");

    FAIL_ON(memsector_save(&mswr) != 0);
    printf("Memsector saved\n");

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

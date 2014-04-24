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

#include <string>

#include "mws/dbc/MemCrawlDb.hpp"
#include "mws/dbc/MemFormulaDb.hpp"
#include "mws/index/MwsIndexNode.hpp"
#include "mws/index/memsector.h"
#include "mws/index/IndexManager.hpp"
#include "mws/index/MeaningDictionary.hpp"
using mws::index::MeaningDictionary;
#include "mws/xmlparser/loadMwsHarvestFromFd.hpp"
#include "common/utils/compiler_defs.h"
#include "common/utils/FlagParser.hpp"
using common::utils::FlagParser;

#include "build-gen/config.h"

#define DEFAULT_TMP_MEMSECTOR_PATH  "/tmp/test.memsector"
#define DEFAULT_MEMSECTOR_SIZE      200 * 1024 * 1024
#define DEFAULT_HARVEST_PATH        MWS_TESTDATA_PATH

using namespace std;
using namespace mws;

const memsector_alloc_header_t *alloc;

struct Tester {
    static inline
    bool memsector_inode_consistent(const MwsIndexNode* tmp_node,
                                    const inode_t* inode) {
        switch (inode->type) {
        case INTERNAL_NODE: {
            if (tmp_node->children.size() != inode->size) return false;
            int i = 0;
            for (auto& kv : tmp_node->children) {
                MeaningId           meaningId  = kv.first.id;
                Arity               arity      = kv.first.arity;
                const MwsIndexNode* child_node = kv.second;

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
            leaf_t *leaf = (leaf_t*)inode;
            return ((tmp_node->id == leaf->formula_id) &&
                    (tmp_node->solutions == leaf->num_hits));
        }

        default: {
            assert(false);
        }
        }
    }
};

static
int test_memsector_consistency(MwsIndexNode* data, memsector_handle_t* ms) {
    alloc = ms_get_alloc(ms);
    if (Tester::memsector_inode_consistent(data, ms->index.root))
        return 0;
    else
        return -1;
}

int main(int argc, char* argv[]) {
    memsector_writer_t mswr;
    memsector_handle_t ms;
    dbc::CrawlDb* crawlDb;
    dbc::FormulaDb* formulaDb;
    MwsIndexNode* data;
    MeaningDictionary* meaningDictionary;
    index::IndexManager* indexManager;
    index::IndexingOptions indexingOptions;
    string harvest_path;
    string tmp_memsector_path;
    uint32_t memsector_size;

    FlagParser::addFlag('s', "memsector-size",          FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('I', "include-harvest-path",    FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('O', "tmp-memsector-path",      FLAG_OPT, ARG_REQ);
    FlagParser::addFlag('c', "enable-ci-renaming",      FLAG_OPT, ARG_REQ);

    if (FlagParser::parse(argc, argv) != 0) {
        PRINT_WARN("%s", FlagParser::getUsage().c_str());
        goto fail;
    }

    if (FlagParser::hasArg('s')) {
        int64_t size = strtoll(FlagParser::getArg('s').c_str(), NULL, 10);
        if (size > 0 && size < UINT32_MAX) {
            memsector_size = size;
        } else {
            PRINT_WARN("Invalid size \"%s\"\n",
                    FlagParser::getArg('s').c_str());
            goto fail;
        }
    } else {
        PRINT_WARN("Using default memsector size %d\n",
                DEFAULT_MEMSECTOR_SIZE);
        memsector_size = DEFAULT_MEMSECTOR_SIZE;
    }

    if (FlagParser::hasArg('I')) {
        harvest_path = FlagParser::getArg('I');
    } else {
        PRINT_WARN("Using default include harvest path %s\n",
                DEFAULT_HARVEST_PATH);
        harvest_path = DEFAULT_HARVEST_PATH;
    }

    if (FlagParser::hasArg('O')) {
        tmp_memsector_path = FlagParser::getArg('O');
    } else {
        PRINT_WARN("Using default temporary memsector path %s\n",
                DEFAULT_TMP_MEMSECTOR_PATH);
        tmp_memsector_path = DEFAULT_TMP_MEMSECTOR_PATH;
    }

    if (FlagParser::hasArg('c')) {
        indexingOptions.renameCi = true;
    } else {
        PRINT_WARN("Not renaming ci\n");
        indexingOptions.renameCi = false;
    }

    crawlDb = new dbc::MemCrawlDb();
    formulaDb = new dbc::MemFormulaDb();
    data = new MwsIndexNode();
    meaningDictionary = new MeaningDictionary();
    indexManager = new index::IndexManager(formulaDb, crawlDb,
                                           data, meaningDictionary,
                                           indexingOptions);

    /* ensure the file does not exist */
    FAIL_ON(unlink(tmp_memsector_path.c_str()) != 0 && errno != ENOENT);

    FAIL_ON(parser::loadMwsHarvestFromDirectory(indexManager,
                                                AbsPath(harvest_path),
                                                ".harvest",
                                                /* recursive = */ false) <= 0);

    FAIL_ON(memsector_create(&mswr, tmp_memsector_path.c_str(),
                             memsector_size) != 0);
    printf("Memsector %s created\n", tmp_memsector_path.c_str());
    
    data->exportToMemsector(&mswr);
    printf("Index exported to memsector\n");
    printf("Space used: %d Kb\n",
           memsector_size_inuse(&mswr.ms_header->alloc_header) / 1024);

    FAIL_ON(memsector_save(&mswr) != 0);
    printf("Memsector saved\n");

    FAIL_ON(memsector_load(&ms, tmp_memsector_path.c_str()) != 0);
    printf("Memsector loaded\n");

    if (test_memsector_consistency(data, &ms) != 0) {
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

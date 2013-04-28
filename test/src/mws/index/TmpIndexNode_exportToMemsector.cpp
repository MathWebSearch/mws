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

#include <string>
#include <cerrno>

#include "mws/index/MwsIndexNode.hpp"
#include "mws/index/memsector.h"
#include "mws/types/MeaningDictionary.hpp"
#include "mws/xmlparser/loadMwsHarvestFromFd.hpp"
#include "common/utils/macro_func.h"
#include "mws/dbc/PageDbHandle.hpp"

#include "config.h"

#define TMPDBENV_PATH   "/tmp"
#define TMPFILE_PATH    "/tmp/test.map"
#define TMPFILE_SIZE    500 * 1024 * 1024


using namespace std;
using namespace mws;

namespace mws {

class Tester {
  public:
    static bool memsector_inode_consistent(MwsIndexNode* tmp_node, inode_t* inode);
};

}

static int test_memsector_consistency(MwsIndexNode* data, memsector_handle_t* ms);

int main() {
    memsector_writer_t mswr;
    memsector_handle_t ms;
    const char *ms_path = TMPFILE_PATH;
    string harvest_path = MWS_TESTDATA_PATH;
    PageDbHandle dbhandle;
    string dbenv_path = TMPDBENV_PATH;

    MwsIndexNode* data = new MwsIndexNode();

    /* ensure the file does not exist */
    FAIL_ON(unlink(ms_path) != 0 && errno != ENOENT);

    FAIL_ON(dbhandle.init(dbenv_path) != 0);
  
    FAIL_ON(loadMwsHarvestFromDirectory(data, AbsPath(harvest_path), &dbhandle) <= 0);

    FAIL_ON(memsector_create(&mswr, ms_path, TMPFILE_SIZE) != 0);
    printf("Memsector %s created\n", ms_path);
    
    data->exportToMemsector(&mswr);
    printf("Index exported to memsector\n");
    printf("Space used: %d Kb\n",
           memsector_size_inuse(&mswr.ms_header->alloc_header) / 1024);

    FAIL_ON(memsector_save(&mswr) != 0);
    printf("Memsector saved\n");

    FAIL_ON(memsector_load(&ms, ms_path) != 0);
    printf("Memsector loaded\n");

    if (test_memsector_consistency(data, &ms) != 0) {
        printf("FAIL: Inconsistency detected!\n");
        goto fail;
    }
    printf("Memsector consistent with index\n");

    FAIL_ON(memsector_remove(&ms) != 0);
    printf("Memsector removed\n");

    dbhandle.clean();

    return 0;

fail:
    dbhandle.clean();
    return -1;
}

const memsector_alloc_header_t *alloc;

bool Tester::memsector_inode_consistent(MwsIndexNode* tmp_node, inode_t* inode) {
    // XXX unbound recursive behavior... dangerous in general... ok in test

    switch(inode->type) {
        case INTERNAL_NODE: {
            if (tmp_node->children.size() != inode->size) return false;

            int i = 0;
            MwsIndexNode::_MapType::iterator it;
            for (it = tmp_node->children.begin();
                it != tmp_node->children.end();
                it++, i++) {

                MeaningId     meaningId  = it->first.first;
                Arity         arity      = it->first.second;
                MwsIndexNode* child_node = it->second;

                if (meaningId != inode->data[i].token.id) return false;
                if (arity     != inode->data[i].token.arity) return false;

                inode_t* child_inode = (inode_t*) memsector_off2addr(alloc, inode->data[i].off);
                if (!memsector_inode_consistent(child_node, child_inode)) return false;
            }
            return true;
        }

        case LEAF_NODE: {
            leaf_t *leaf = (leaf_t*)inode;
            return (tmp_node->id == leaf->dbid) && (tmp_node->solutions == leaf->num_hits);
        }

        default: {
            assert(false);
        }
    }
}

static
int test_memsector_consistency(MwsIndexNode* data, memsector_handle_t* ms) {
    alloc = ms_get_alloc(ms);
    if (Tester::memsector_inode_consistent(data, ms->index.root))
        return 0;
    else
        return -1;
}

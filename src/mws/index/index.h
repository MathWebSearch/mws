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
 * @brief   Compact index data structure
 * @file    index.h
 * @date    24 Jan 2013
 *
 * License: GPLv3
 */

#ifndef __MWS_INDEX_INDEX_H
#define __MWS_INDEX_INDEX_H

// System includes

#include <assert.h>
#include <stdint.h>
#include <string.h>

// Local includes

#include "common/utils/compiler_defs.h"
#include "mws/index/encoded_token.h"
#include "mws/index/memsector.h"

/*--------------------------------------------------------------------------*/
/* Type declarations                                                        */
/*--------------------------------------------------------------------------*/

/**
 * @brief Index node types
 */
typedef enum node_type_e {
    INTERNAL_NODE = 1,
    LONG_INTERNAL_NODE = 2,
    LEAF_NODE = 3
} node_type_t;

/**
 * @brief encoded_token -> offset_ptr pair
 */
struct encoded_token_dict_entry_s {
    encoded_token_t token;
    memsector_off_t off;
} PACKED;
typedef struct encoded_token_dict_entry_s encoded_token_dict_entry_t;

struct encoded_token_dict_entry_long_s {
    encoded_token_t token;
    memsector_long_off_t off;
} PACKED;
typedef struct encoded_token_dict_entry_long_s encoded_token_dict_entry_long_t;

/**
 * @brief Internal index node
 */
struct inode_s {
    node_type_t type : 2; /* should be INTERNAL_NODE */
    uint64_t size : 62;
    encoded_token_dict_entry_t data[];
} PACKED;
typedef struct inode_s inode_t;

struct inode_long_s {
    node_type_t type : 2; /* should be LONG_INTERNAL_NODE */
    uint64_t size : 62;
    encoded_token_dict_entry_long_t data[];
} PACKED;
typedef struct inode_long_s inode_long_t;

/**
 * @brief Leaf index node
 */
struct leaf_s {
    node_type_t type : 2; /* should be LEAF_NODE */
    uint32_t num_hits : 30;
    uint32_t formula_id : 32;
} PACKED;
typedef struct leaf_s leaf_t;

/**
 * Index in-memory header
 */
struct index_header_s {
    inode_t root;
} PACKED;
typedef struct index_header_s index_header_t;

typedef struct index_handle_s {
    const inode_long_t* root;
    memsector_header_t* ms;
} index_handle_t;

/*--------------------------------------------------------------------------*/
/* Methods                                                                  */
/*--------------------------------------------------------------------------*/

BEGIN_DECLS

static inline uint32_t memsector_inode_size(uint32_t num_children) {
    return sizeof(inode_t) + num_children * sizeof(encoded_token_dict_entry_t);
}

static inline uint32_t memsector_leaf_size(void) { return sizeof(leaf_t); }

static inline memsector_long_off_t memsector_write_inode_begin(
    memsector_writer_t* msw, uint32_t num_children,
    memsector_long_off_t furthermost_child_off) {
    // No inode write should be in progress
    assert(msw->inode.entries_delivered == 0);
    assert(msw->inode.entries_promised == 0);
    msw->inode.entries_promised = num_children;

    memsector_long_off_t currOff = memsector_get_current_offset(msw);

    memsector_long_off_t maxRelOff = currOff - furthermost_child_off;
    msw->inode.has_long_offsets = maxRelOff >= MEMSECTOR_LONG_OFF_START;

    if (msw->inode.has_long_offsets) {
        inode_long_t inode;
        inode.type = LONG_INTERNAL_NODE;
        inode.size = num_children;
        memsector_write(msw, &inode, sizeof(inode));
    } else {
        inode_t inode;
        inode.type = INTERNAL_NODE;
        inode.size = num_children;
        memsector_write(msw, &inode, sizeof(inode));
    }

    return currOff;
}

static inline void memsector_write_inode_encoded_token_entry(
    memsector_writer_t* msw, encoded_token_t encoded_token,
    memsector_long_off_t off) {
    assert(msw->inode.entries_promised > msw->inode.entries_delivered++);
    assert(off != MEMSECTOR_OFF_NULL);

    if (msw->inode.has_long_offsets) {
        encoded_token_dict_entry_long_t entry;
        entry.token = encoded_token;
        entry.off = off;
        memsector_write(msw, &entry, sizeof(entry));
    } else {
        encoded_token_dict_entry_t entry;
        entry.token = encoded_token;
        entry.off = off;
        memsector_write(msw, &entry, sizeof(entry));
    }
}

static inline void memsector_write_inode_end(memsector_writer_t* msw) {
    assert(msw->inode.entries_delivered > 0);
    assert(msw->inode.entries_delivered == msw->inode.entries_promised);
    msw->inode.entries_delivered = 0;
    msw->inode.entries_promised = 0;
}

static inline memsector_long_off_t memsector_write_leaf(
    memsector_writer_t* mswr, uint32_t num_hits, uint32_t formula_id) {
    memsector_long_off_t off = memsector_get_current_offset(mswr);

    // No inode write should be in progress
    assert(mswr->inode.entries_delivered == 0);
    assert(mswr->inode.entries_promised == 0);

    leaf_t leaf;
    leaf.type = LEAF_NODE;
    leaf.num_hits = num_hits;
    leaf.formula_id = formula_id;
    memsector_write(mswr, &leaf, sizeof(leaf));

    return off;
}

static inline const inode_long_t* memsector_get_root(
    memsector_handle_t* msHandle) {
    return (inode_long_t*)memsector_off2addr(msHandle->ms,
                                             msHandle->ms->root_off);
}

static inline memsector_off_t inode_get_child(const inode_t* inode,
                                              encoded_token_t token) {
    int32_t left, right;

    left = 0;
    right = inode->size - 1;

    while (left <= right) {
        int32_t center = left + (right - left) / 2;
        int result = memcmp(&inode->data[center].token, &token, sizeof(token));
        if (result > 0) {
            right = center - 1;
        } else if (result == 0) {
            return inode->data[center].off;
        } else {
            left = center + 1;
        }
    }

    return MEMSECTOR_OFF_NULL;
}

static inline memsector_long_off_t inode_long_get_child(
    const inode_long_t* inode, encoded_token_t token) {
    int32_t left, right;

    left = 0;
    right = inode->size - 1;

    while (left <= right) {
        int32_t center = left + (right - left) / 2;
        int result = memcmp(&inode->data[center].token, &token, sizeof(token));
        if (result > 0) {
            right = center - 1;
        } else if (result == 0) {
            return inode->data[center].off;
        } else {
            left = center + 1;
        }
    }

    return MEMSECTOR_OFF_NULL;
}

static inline uint32_t inode_get_max_var(const inode_t* inode) {
    uint32_t i = 0;
    while (inode->data[i].token.id <= VAR_ID_MAX) i++;

    return i;
}

static inline memsector_off_t inode_get_qvar(const inode_t* inode,
                                             uint32_t qvar_id) {
    assert(inode->data[qvar_id].token.id == qvar_id);

    return inode->data[qvar_id].off;
}

END_DECLS

#endif  // __MWS_INDEX_INDEX_H

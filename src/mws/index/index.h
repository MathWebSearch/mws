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

#include <stdint.h>
#include <string.h>

// Local includes

#include "common/utils/compiler_defs.h"
#include "mws/index/encoded_token.h"
#include "mws/index/memsector_allocator.h"

/*--------------------------------------------------------------------------*/
/* Type declarations                                                        */
/*--------------------------------------------------------------------------*/

/**
 * @brief Index node types
 */
typedef enum node_type_e {
    INTERNAL_NODE   = 1,
    LEAF_NODE       = 2
} node_type_t;

/**
 * @brief Internal index node
 */
struct inode_s {
    node_type_t type    : 2;  /* should be INTERNAL_NODE */
    uint32_t    size    : 30;
    encoded_token_dict_entry_t data[];
} PACKED;
typedef struct inode_s inode_t;

/**
 * @brief Leaf index node
 */
struct leaf_s {
    node_type_t type    : 2;  /* should be LEAF_NODE */
    uint32_t num_hits   : 30;
    uint32_t formula_id : 32;
} PACKED;
typedef struct leaf_s leaf_t;

/**
 * Index in-memory header
 */
struct index_header_s {
    inode_t  root;
} PACKED;
typedef struct index_header_s index_header_t;

typedef struct index_handle_s {
    inode_t *root;
    memsector_alloc_header_t *alloc;
} index_handle_t;

/*--------------------------------------------------------------------------*/
/* Methods                                                                  */
/*--------------------------------------------------------------------------*/

BEGIN_DECLS

static inline
uint32_t inode_size(uint32_t num_children) {
    return sizeof(inode_t) + num_children * sizeof(encoded_token_dict_entry_t);
}

static inline
uint32_t leaf_size(void) {
    return sizeof(leaf_t);
}

static inline
memsector_off_t inode_alloc(memsector_alloc_header_t* alloc,
                            uint32_t num_children) {
    return memsector_alloc(alloc, inode_size(num_children));
}

static inline
memsector_off_t leaf_alloc(memsector_alloc_header_t* alloc) {
    return memsector_alloc(alloc, leaf_size());
}

static inline
memsector_off_t inode_get_child(const inode_t* inode, encoded_token_t token) {
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

static inline
uint32_t inode_get_max_var(const inode_t* inode) {
    uint32_t i = 0;
    while (inode->data[i].token.id <= VAR_ID_MAX) i++;

    return i;
}

static inline
memsector_off_t inode_get_qvar(const inode_t* inode, uint32_t qvar_id) {
    assert(inode->data[qvar_id].token.id == qvar_id);

    return inode->data[qvar_id].off;
}

END_DECLS

#endif // __MWS_INDEX_INDEX_H

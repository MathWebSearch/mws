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
 * @brief   Memory sector handle
 * @file    memsector.h
 * @date    01 Feb 2013
 *
 * License: GPLv3
 */

#ifndef __MWS_INDEX_MEMSECTOR_H
#define __MWS_INDEX_MEMSECTOR_H

// System includes

#include <assert.h>
#include <stdint.h>

// Local includes

#include "common/utils/compiler_defs.h"
#include "common/utils/mmap.h"
#include "mws/index/encoded_token.h"

/*--------------------------------------------------------------------------*/
/* Type declarations                                                        */
/*--------------------------------------------------------------------------*/

/**
 * Compact offset pointer
 */
typedef uint32_t memsector_off_t;
typedef uint64_t memsector_long_off_t;

#define MEMSECTOR_OFF_NULL      (memsector_off_t) 0
#define MEMSECTOR_ALLOC_UNIT    (uint32_t) 4
#define MEMSECTOR_LONG_OFF_START (1ULL << 32)

/**
 * @brief Memsector header
 */
struct memsector_header_s {
    uint32_t magic;
    uint32_t version;
    uint64_t index_size;
    uint32_t checksum;
    memsector_long_off_t root_off;
} PACKED;
typedef struct memsector_header_s memsector_header_t;

typedef struct memsector_handle_s {
    mmap_handle_t mmap_handle;
    memsector_header_t* ms;
} memsector_handle_t;

typedef struct memsector_writer_s {
    FILE* file;
    memsector_header_t ms;
    uint64_t offset;
    struct {
        uint32_t entries_promised;
        uint32_t entries_delivered;
        bool has_long_offsets;
    } inode;
} memsector_writer_t;

/*--------------------------------------------------------------------------*/
/* Methods                                                                  */
/*--------------------------------------------------------------------------*/

BEGIN_DECLS

static inline
const char* memsector_off2addr(const memsector_header_t* ms,
                               memsector_long_off_t off) {
    return ((char*)ms) + MEMSECTOR_ALLOC_UNIT * off;
}

static inline
const char* memsector_relOff2addr(const char* baseAddr,
                                  memsector_long_off_t off) {
    return baseAddr - MEMSECTOR_ALLOC_UNIT * off;
}

static inline
memsector_off_t memsector_get_current_offset(const memsector_writer_t* mswr) {
    assert(mswr->offset % MEMSECTOR_ALLOC_UNIT == 0);
    return mswr->offset / MEMSECTOR_ALLOC_UNIT;
}

/**
 * @return 0 on success, -1 on failure.
 */
int memsector_create(memsector_writer_t *mswr,
                     const char *path);

void memsector_write(memsector_writer_t* msw,
                     void* data, size_t size);

/**
 * @return 0 on success, -1 on failure.
 */
int memsector_save(memsector_writer_t *msw, memsector_long_off_t index_off);

/**
 * @return 0 on success, -1 on failure.
 */
int memsector_load(memsector_handle_t *ms, const char *path);

/**
 * @return 0 on success, -1 on failure.
 */
int memsector_unload(memsector_handle_t *ms);

/**
 * @return 0 on success, -1 on failure.
 */
int memsector_remove(memsector_handle_t *ms);

END_DECLS

#endif // __MWS_INDEX_MEMSECTOR_H

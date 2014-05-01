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

#include <stdint.h>

// Local includes

#include "common/utils/compiler_defs.h"
#include "common/utils/mmap.h"
#include "mws/index/memsector_allocator.h"
#include "mws/index/encoded_token.h"
#include "mws/index/index.h"

/*--------------------------------------------------------------------------*/
/* Type declarations                                                        */
/*--------------------------------------------------------------------------*/

struct memsector_header_s {
    memsector_alloc_header_t alloc_header;
    uint32_t index_header_off;
    uint32_t signature; // TODO
} PACKED;
typedef struct memsector_header_s memsector_header_t;

typedef struct memsector_handle_s {
    mmap_handle_t mmap_handle;
    memsector_alloc_header_t* alloc;
    index_handle_t index;
} memsector_handle_t;

typedef struct memsector_writer_s {
    mmap_handle_t mmap_handle;
    memsector_header_t* ms_header;
} memsector_writer_t;

/*--------------------------------------------------------------------------*/
/* Methods                                                                  */
/*--------------------------------------------------------------------------*/

BEGIN_DECLS

/**
 * @return 0 on success, -1 on failure.
 */
int memsector_create(memsector_writer_t *msw,
                     const char *path,
                     uint32_t size);

/**
 * @return 0 on success, -1 on failure.
 */
int memsector_save(memsector_writer_t *msw);

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

/**
 * @return pointer to the allocation header
 */
static inline
memsector_alloc_header_t *mswr_get_alloc(memsector_writer_t* mswr) {
    return &mswr->ms_header->alloc_header;
}

static inline
memsector_alloc_header_t* ms_get_alloc(memsector_handle_t* ms) {
    return ms->alloc;
}

END_DECLS

#endif // __MWS_INDEX_MEMSECTOR_H

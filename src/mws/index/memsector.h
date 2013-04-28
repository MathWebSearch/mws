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
#include "mws/index/encoded_token_dict.h"
#include "mws/index/encoded_url_dict.h"
#include "mws/index/index.h"

/*--------------------------------------------------------------------------*/
/* Type declarations                                                        */
/*--------------------------------------------------------------------------*/

struct memsector_header_s {
    memsector_alloc_header_t alloc_header;
    uint32_t index_header_off;
    uint32_t encoded_token_dict_header_off;
    uint32_t encoded_url_dict_header_off;
    uint32_t signature; // TODO
} PACKED;
typedef struct memsector_header_s memsector_header_t;

typedef struct memsector_handle_s {
    mmap_handle_t mmap_handle;
    memsector_alloc_header_t* alloc;
    index_handle_t index;
    //encoded_token_dict_handle_t encoded_token_dict;
    //encoded_url_dict_handle_t encoded_url_dict;
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

/**
 * Prepare the memsector to begin writing the index
 */
static inline
void mswr_index_begin(memsector_writer_t* RESTRICT mswr) {
    memsector_header_t*       ms    = mswr->ms_header;
    memsector_alloc_header_t* alloc = &ms->alloc_header;

    /* set index header offset */
    memsector_off_t index_off = memsector_alloc_get_curr_off(alloc);
    ms->index_header_off = index_off;
}

/**
 * Prepare the memsector to begin writing the encoded token dictionary
 *
 * @return pointer to the encoded token dictionary header
 */
static inline
encoded_token_dict_header_t*
mswr_encoded_token_dict_begin(memsector_writer_t* RESTRICT mswr) {
    memsector_header_t*       ms    = mswr->ms_header;
    memsector_alloc_header_t* alloc = &ms->alloc_header;

    /* alloc encoded_token_dict header */
    memsector_off_t encoded_token_dict_header_off =
            memsector_alloc(alloc, sizeof(encoded_token_dict_header_t));
    ms->encoded_token_dict_header_off = encoded_token_dict_header_off;

    return (encoded_token_dict_header_t*)
            memsector_off2addr(alloc, encoded_token_dict_header_off);
}

/**
 * Prepare the memsector to begin writing the URL dictionary
 *
 * @return pointer to the URL dictionary header
 */
static inline
encoded_url_dict_header_t*
mswr_encoded_url_dict_begin(memsector_writer_t* RESTRICT mswr) {
    memsector_header_t*       ms    = mswr->ms_header;
    memsector_alloc_header_t* alloc = &ms->alloc_header;

    /* alloc encoded_url_dict header */
    memsector_off_t encoded_url_dict_header_off =
            memsector_alloc(alloc, sizeof(encoded_url_dict_header_t));
    ms->encoded_url_dict_header_off = encoded_url_dict_header_off;

    return (encoded_url_dict_header_t*) 
            memsector_off2addr(alloc, encoded_url_dict_header_off);
}

END_DECLS

#endif // __MWS_INDEX_MEMSECTOR_H

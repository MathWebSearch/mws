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
 * @file    memsector.c
 * @date    01 Feb 2013
 *
 * License: GPLv3
 */

// System includes

#include <stdint.h>
#include <string.h>

// Local includes

#include "memsector.h"

/*--------------------------------------------------------------------------*/
/* Implementation                                                           */
/*--------------------------------------------------------------------------*/

int memsector_create(memsector_writer_t *msw,
                     const char *path,
                     uint32_t size) {
    int status;

    /* create and mmap memsector file */
    status = mmap_create(path, size,
                         MAP_SHARED,
                         &msw->mmap_handle);
    if (status == -1) return -1;

    /* initialize memory allocator */
    memsector_header_t ms;

    ms.alloc_header.curr_offset = sizeof(memsector_header_t);
    ms.alloc_header.end_offset = size;
    memcpy(msw->mmap_handle.start_addr, &ms, sizeof(memsector_header_t));
    msw->ms_header = (memsector_header_t*) msw->mmap_handle.start_addr;

    return 0;
}

int memsector_save(memsector_writer_t *msw) {
    return mmap_unload(&msw->mmap_handle);
}

int memsector_load(memsector_handle_t *ms, const char *path) {
    int status;

    // mmap_handle
    status = mmap_load(path, MAP_SHARED, &ms->mmap_handle);
    if (status == -1) return -1;

    // alloc
    memsector_header_t* memsector_header =
            (memsector_header_t*) ms->mmap_handle.start_addr;
    ms->alloc = &memsector_header->alloc_header;

    // index handle
    index_header_t* index_header = (index_header_t*)
            memsector_off2addr(ms->alloc, memsector_header->index_header_off);
    ms->index.alloc = ms->alloc;
    ms->index.root  = &index_header->root;

    return 0;
}

int memsector_unload(memsector_handle_t* ms) {
    return mmap_unload(&ms->mmap_handle);
}

int memsector_remove(memsector_handle_t *ms) {
    return mmap_remove(&ms->mmap_handle);
}

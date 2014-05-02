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
 * @brief   Memory sector
 * @file    memsector.c
 * @date    01 Feb 2013
 *
 * License: GPLv3
 */

#include <stdint.h>
#include <string.h>

#include "common/utils/mmap.h"
#include "mws/index/memsector.h"

#include "crc32/crc32.h"

/*--------------------------------------------------------------------------*/
/* Local methods                                                            */
/*--------------------------------------------------------------------------*/

static uint32_t memsector_get_checksum(const memsector_header_t* memsector);

/*--------------------------------------------------------------------------*/
/* Implementation                                                           */
/*--------------------------------------------------------------------------*/

int memsector_create(memsector_writer_t *msw,
                     const char *path,
                     uint32_t size) {
    size_t real_size = sizeof(memsector_header_t) + size;
    int status;

    /* create and mmap memsector file */
    status = mmap_create(path, real_size,
                         MAP_SHARED,
                         &msw->mmap_handle);
    if (status == -1) return -1;

    /* initialize header */
    memsector_header_t ms;
    ms.alloc_header.curr_offset = sizeof(memsector_header_t);
    ms.alloc_header.end_offset = real_size;
    ms.index_header_off = memsector_alloc_get_curr_off(&ms.alloc_header);

    /* copy header to memsector file */
    memcpy(msw->mmap_handle.start_addr, &ms, sizeof(memsector_header_t));
    msw->ms_header = (memsector_header_t*) msw->mmap_handle.start_addr;

    return 0;
}

int memsector_save(memsector_writer_t *msw) {
    msw->ms_header->checksum = memsector_get_checksum(msw->ms_header);
    return mmap_unload(&msw->mmap_handle);
}

int memsector_load(memsector_handle_t *ms, const char *path) {
    int status;

    // load memory map
    status = mmap_load(path, MAP_SHARED, &ms->mmap_handle);
    if (status == -1) {
        return -1;
    }

    // initialize allocator header
    memsector_header_t* memsector =
            (memsector_header_t*) ms->mmap_handle.start_addr;
    ms->alloc = &memsector->alloc_header;

    // check data integrity
    if (memsector->checksum != memsector_get_checksum(memsector)) {
        PRINT_WARN("Memsector %s is corrupted (checksum mismatch)\n", path);
        mmap_unload(&ms->mmap_handle);
        return -1;
    }

    // index handle
    index_header_t* index_header = (index_header_t*)
            memsector_off2addr(ms->alloc, memsector->index_header_off);
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

/*--------------------------------------------------------------------------*/
/* Local implementation                                                     */
/*--------------------------------------------------------------------------*/

static uint32_t memsector_get_checksum(const memsector_header_t* memsector) {
    uint32_t checksum = 0;

    /* hash allocator header */
    checksum = crc32(checksum, &memsector->alloc_header,
                     sizeof(memsector_alloc_header_t));

    /* hash index data */
    const void* index_start = memsector_off2addr(&memsector->alloc_header,
                                                 memsector->index_header_off);
    size_t index_size = memsector_size_inuse(&memsector->alloc_header)
            - sizeof(memsector_header_t);
    checksum = crc32(checksum, index_start, index_size);

    return checksum;
}

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

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <crc32/crc32.h>

#include "common/utils/mmap.h"
#include "mws/index/memsector.h"

const uint64_t MEMSECTOR_MAGIC = 0x88CAFE88;
const uint32_t MEMSECTOR_VERSION = 1;

/*--------------------------------------------------------------------------*/
/* Local methods                                                            */
/*--------------------------------------------------------------------------*/

static uint32_t memsector_get_checksum(const memsector_header_t* memsector);

/*--------------------------------------------------------------------------*/
/* Implementation                                                           */
/*--------------------------------------------------------------------------*/

int memsector_create(memsector_writer_t *mswr,
                     const char *path) {
    /* initialize memsector writer */
    memset(mswr, 0, sizeof(*mswr));

    /* create and resize file */
    mswr->file = fopen(path, "w");
    if (mswr->file == 0) {
        PRINT_WARN("Error while opening %s: %s\n", path, strerror(errno));
        return -1;
    }

    /* initialize header */
    mswr->ms.magic = MEMSECTOR_MAGIC;
    mswr->ms.version = MEMSECTOR_VERSION;
    mswr->offset = sizeof(mswr->ms);

    /* write header to memsector file */
    size_t items_writen = fwrite(&mswr->ms, sizeof(mswr->ms), 1, mswr->file);
    if (items_writen != 1) {
        PRINT_WARN("Error while writing memsector header: %s\n",
                   strerror(errno));
        fclose(mswr->file);
        return -1;
    }

    return 0;
}

void memsector_write(memsector_writer_t* msw,
                     void* data, size_t size) {
    size_t items_writen = fwrite(data, size, 1, msw->file);
    if (items_writen != 1) {
        perror("fwrite");
    }
    msw->offset += size;
    msw->ms.checksum = crc32(msw->ms.checksum, data, size);
}

int memsector_save(memsector_writer_t *msw, memsector_off_t index_off) {
    msw->ms.root_off = index_off;
    msw->ms.index_size = msw->offset - sizeof(msw->ms);
    fseek(msw->file, 0, SEEK_SET);
    fwrite(&msw->ms, sizeof(msw->ms), 1, msw->file);

    return fclose(msw->file);
}

int memsector_load(memsector_handle_t *ms, const char *path) {
    int status;

    // load memory map
    status = mmap_load(path, MAP_SHARED, &ms->mmap_handle);
    if (status == -1) {
        return -1;
    }

    // read and validate memsector data
    memsector_header_t* memsector =
            (memsector_header_t*) ms->mmap_handle.start_addr;
    if (memsector->magic != MEMSECTOR_MAGIC) {
        PRINT_WARN("File %s is not a memsector (magic mismatch)\n", path);
        mmap_unload(&ms->mmap_handle);
        return -1;
    }
    if (memsector->version != MEMSECTOR_VERSION) {
        PRINT_WARN("Cannot process memsector %s v%d\n",
                   path, (int) memsector->version);
        mmap_unload(&ms->mmap_handle);
        return -1;
    }
    if (memsector->checksum != memsector_get_checksum(memsector)) {
        PRINT_WARN("Memsector %s is corrupted (checksum mismatch)\n", path);
        mmap_unload(&ms->mmap_handle);
        return -1;
    }

    // initialize handle data
    ms->ms = memsector;

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
    /* hash index data */
    const char* index_start = ((char*) memsector) + sizeof(*memsector);
    size_t index_size = memsector->index_size;

    return crc32(/* initial crc32 = */ 0, index_start, index_size);
}

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
 * @brief   Memory sector allocator
 * @file    memsector_allocator.h
 * @date    30 Jan 2013
 *
 * License: GPLv3
 */

#ifndef __MWS_INDEX_MEMSECTOR_ALLOCATOR_H
#define __MWS_INDEX_MEMSECTOR_ALLOCATOR_H

// System includes

#include <assert.h>
#include <stdint.h>

#include "common/utils/compiler_defs.h"

/*--------------------------------------------------------------------------*/
/* Type declarations                                                        */
/*--------------------------------------------------------------------------*/

/**
 * Compact offset pointer
 */
typedef uint32_t memsector_off_t;
#define MEMSECTOR_OFF_NULL      0UL
#define MEMSECTOR_ALLOC_UNIT    8ULL

struct memsector_alloc_header_s {
    uint64_t curr_offset;
    uint64_t end_offset;
} PACKED;
typedef struct memsector_alloc_header_s memsector_alloc_header_t;

/*--------------------------------------------------------------------------*/
/* Methods                                                                  */
/*--------------------------------------------------------------------------*/

BEGIN_DECLS

static inline
memsector_off_t memsector_alloc_get_curr_off(
        const memsector_alloc_header_t* alloc) {
    return alloc->curr_offset / MEMSECTOR_ALLOC_UNIT;
}

static inline
uint64_t memsector_size_inuse(const memsector_alloc_header_t* alloc) {
    return alloc->curr_offset;
}

static inline
memsector_off_t memsector_alloc(memsector_alloc_header_t *alloc,
                                uint64_t nbytes) {
    assert(alloc->end_offset - alloc->curr_offset >= nbytes);
    assert(alloc->curr_offset % MEMSECTOR_ALLOC_UNIT == 0);
    memsector_off_t result = memsector_alloc_get_curr_off(alloc);
    alloc->curr_offset += nbytes;

    return result;
}

static inline
void* memsector_off2addr(const memsector_alloc_header_t* alloc,
                         memsector_off_t off) {
    return (void*) (((char*)alloc) + MEMSECTOR_ALLOC_UNIT * off);
}

END_DECLS

#endif // __MWS_INDEX_MEMSECTOR_MEMSECTOR_ALLOCATOR_H

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
 * @brief   Memory map utilities
 * @file    mmap.h
 * @date    04 Feb 2013
 *
 * License: GPLv3
 */

#ifndef __COMMON_UTILS_MMAP_H
#define __COMMON_UTILS_MMAP_H

#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common/utils/compiler_defs.h"

/*--------------------------------------------------------------------------*/
/* Type declarations                                                        */
/*--------------------------------------------------------------------------*/

typedef struct mmap_handle_s {
    const char* path;
    char*    start_addr;
    uint32_t size;
} mmap_handle_t;

/*--------------------------------------------------------------------------*/
/* Methods                                                                  */
/*--------------------------------------------------------------------------*/

BEGIN_DECLS

/**
 * Create and mmap a read/writeable file
 *
 * @return 0 on success
 * @return -1 on failure
 */
int mmap_create(const char* path, off_t size, int flags, mmap_handle_t* mmap);

/**
 * Load a read-only mmap of a file
 *
 * @return 0 on success
 * @return -1 on failure
 */
int mmap_load(const char* path, int flags, mmap_handle_t* mmap);

/**
 * @return 0 on success
 * @return -1 on failure
 */
int mmap_unload(mmap_handle_t* mmap_handle);

/**
 * @return 0 on success
 * @return -1 on failure
 */
int mmap_remove(mmap_handle_t* mmap_handle);

END_DECLS

#endif // __COMMON_UTILS_MMAP_H
